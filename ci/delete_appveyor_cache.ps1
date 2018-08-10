$tokenApp = <appveyor_token> 
$headersAppveyor = @{ "Authorization" = "Bearer $tokenApp"} 
$body = @{
accountName=<account_name> 
projectSlug="pxcore"

} 

$bodyAsJson = $body | ConvertTo-json 

Invoke-WebRequest -Headers $headersAppveyor -Method Delete 'https://ci.appveyor.com/api/projects/$accountName/pxcore/buildcache' 

