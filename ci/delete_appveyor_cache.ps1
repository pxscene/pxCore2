if ( $args.count -ne 2 ) {
"Usage ./delete_appveyor_cache.ps1 <API_TOKEN> <ACCOUNT NAME>"
exit
}

$tokenApp = [string]$args[0]
$accountName = [string]$args[1] 
$headersAppveyor = @{ "Authorization" = "Bearer $tokenApp"} 
$body = @{
accountName=$accountName
projectSlug="pxcore"

} 

$bodyAsJson = $body | ConvertTo-json 

Invoke-WebRequest -Headers $headersAppveyor -Method Delete 'https://ci.appveyor.com/api/projects/$accountName/pxcore/buildcache' 
