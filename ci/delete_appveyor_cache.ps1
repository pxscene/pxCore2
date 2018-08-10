$tokenApp = [string]$args[0]
$headersAppveyor = @{ "Authorization" = "Bearer $tokenApp"} 
$body = @{
accountName="pxscene"
projectSlug="pxcore"

} 

$bodyAsJson = $body | ConvertTo-json 

Invoke-WebRequest -Headers $headersAppveyor -Method Delete 'https://ci.appveyor.com/api/projects/pxscene/pxcore/buildcache' 
