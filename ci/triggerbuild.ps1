
$commitIdVal=[string]$args[0]
$tokenApp = [string]$args[1]
if (($commitIdVal.length -ne 0) -and  ($tokenApp.length -ne 0))
{ 
Write-Host "`nTriggering build for commit " $commitIdVal "..."
}
else 
{ 
Write-Host "`nPlease input proper commitId and appveyor API token." 
Write-Host "Usage : .\triggerbuild.ps1 <commitId> <API_TOKEN>`n"
return 
}

$headersAppveyor = @{ "Authorization" = "Bearer $tokenApp"} 
$body = @{
accountName="pxscene" 
projectSlug="pxcore"
branch="master"
commitId=$commitIdVal
} 

$bodyAsJson = $body | ConvertTo-json 

Invoke-Restmethod -uri 'https://ci.appveyor.com/api/builds' -Headers $headersAppveyor -Method POST  -Body $body 

