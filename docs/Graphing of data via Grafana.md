# Graphing of data via Grafana

## Overview
Performance results are stored in a hosted PostGRES DB and data can then be viewed via Grafana or other graphing tools. For this document it is assumed that this is hosted in Azure, but any other cloud provide should work. Steps in this document use Azure CLI, but other tools would be needed for different Cloud Providers.

# Gather required data
There are several pieces of data needed before this can be setup.
1. Tenant ID for your AAD account -> $TENANT_ID
2. User ID for your AAD account -> $USER_ID
3. Subscription for your AAD account -> $SUBSCRIPTION_ID
4. Github organization you want to upload results from -> $GITHUB_ORG
5. Database host name you want to use -> $DB_HOST
6. Strong password for your DB -> $DB_PASSWORD.
7. Location where you want to host -> $LOCATION
8. Web Service name -> $GRAFANA_HOST

Note:
$GRAFANA_HOST and $DB_HOST need to be globally unique within the Azure namespace.

## Log into Azure Subscription
This step assumes there is an existing Azure subscription:

```
az login
az account set --subscription $SUBSCRIPTION_ID
az configure --defaults location=$LOCATION
```

Note:
Locations can be listed via:
```
az account list-locations
```

## Create resource group to contain the setup (permits easier monitoring and tracking of costs)

```
az group create --resource-group bpf_performance
```

## Create identity to upload perf data
```
az identity create --name uploader --resource-group bpf_performance
az identity federated-credential create --name github --identity-name uploader --resource-group bpf_performance --issuer https://token.actions.githubusercontent.com --subject repo:$GITHUB_ORG/bpf_performance:ref:refs/heads/main

```

Record the PrincipalID from the response as $UPLOADER_PRINCIPAL_ID and ClientID as $UPLOADER_CLIENT_ID.

Assign reader permission to this managed identity.
```
az role assignment create --assignee-object-id $UPLOADER_PRINCIPAL_ID --assignee-principal-type ServicePrincipal --role Reader --scope /subscriptions/$SUBSCRIPTION_ID/resourcegroups/bpf_performance
```

## Set secrets in GitHub
Add the following repository secrets under: "Settings -> Secrets and Variables -> Actions"
AZURE_CLIENT_ID = $UPLOADER_CLIENT_ID
AZURE_SUBSCRIPTION_ID = $SUBSCRIPTION_ID
AZURE_TENANT_ID = $TENANT_ID

## Create DB
```
az postgres flexible-server create --resource-group bpf_performance --active-directory-auth Enabled --database-name bpf_performance --name $DB_HOST --password-auth Enabled --admin-user postgres --admin-password $DB_PASSWORD --sku-name Standard_B1ms --tier Burstable --yes
```

Then:
Update .github/workflows/UploadPerfResults.yml with the name of your some_unique_name.

## Add firewall rule to allow Azure Services to access the DB

```
az postgres flexible-server firewall-rule create --resource-group bpf_performance --name $DB_HOST --rule-name allowazureservices --start-ip-address 0.0.0.0
```

## Add yourself as admin
```
az postgres flexible-server ad-admin create --resource-group bpf_performance --display-name admin --object-id $USER_ID --server-name bpf_performance
```

## Create the required tables
```bash
export PGUSER=admin
export PGPASSWORD=$(az account get-access-token --resource-type oss-rdbms --query "[accessToken]" -o tsv)
export PGHOST=$DB_HOST.postgres.database.azure.com
export PGDATABASE=bpf_performance
psql -f scripts/create_table_postfgres.sql
```

## Add db_user to postgres and grant permissions
```bash
export PGUSER=admin
export PGPASSWORD=$(az account get-access-token --resource-type oss-rdbms --query "[accessToken]" -o tsv)
export PGHOST=$DB_HOST.postgres.database.azure.com
export PGDATABASE=postgres
psql -c "select * from pgaadauth_create_principal_with_oid('uploader', '$UPLOADER_PRINCIPAL_ID', 'service', false, false);"
export PGDATABASE=bpf_performance
psql -c "GRANT INSERT ON TABLE BenchmarkResults TO uploader;"
psql -c "GRANT ALL PRIVILEGES ON TABLE BenchmarkResults TO postgres;"
```

## Create app service plan and web app

First update grafana/docker-compose.yml with setting GF_DATABASE_HOST = $DB_HOST.postgres.database.azure.com

```
az appservice plan create --name bpf_performance_grafana --resource-group bpf_performance --is-linux --sku B1
az webapp create --name $GRAFANA_HOST --resource-group bpf_performance --plan bpf_performance_grafana --https-only --multicontainer-config-file grafana/docker-compose.yml --multicontainer-config-type COMPOSE --public-network-access enabled
az webapp config appsettings set --name $GRAFANA_HOST --resource-group bpf_performance --settings GF_DATABASE_PASSWORD=$DB_PASSWORD
```

## Connect to website and complete setup

1. Connect to the website https://$GRAFANA_HOST.azurewebsites.net/
2. Login with default user name and password (admin), then change the password.
3. Home -> Connections -> Add new connection -> Postgres (fill in fields).
4. Import dashboards from grafana/*.json and customize.
