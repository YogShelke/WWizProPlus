#disable iis feature 

#check if os is Windows 7 or not
$OSVersionDetails = (gwmi win32_operatingsystem).caption
$OSVersionArr = $OSVersionDetails.split(" ");
$OSVersion = "";
For($index = 0; $index -lt $OSVersionArr.length-2; $index++)
{
	$OSVersion += $OSVersionArr[$index]
}
if($OSVersion -eq "MicrosoftWindows7")
{
	dism /online /Disable-Feature /FeatureName:IIS-WebServerRole /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-FTPServer /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-FTPSvc /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-FTPExtensibility /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-WebServerManagementTools /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-ManagementConsole /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-ManagementScriptingTools /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-ManagementService /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-WebServer /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-CommonHttpFeatures /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-HttpErrors /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-StaticContent /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-HttpRedirect /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-WebDAV /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-HealthAndDiagnostics /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-CustomLogging /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-HttpLogging /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-RequestMonitor /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-HttpTracing /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-LoggingLibraries /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-ODBCLogging /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-Performance /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-HttpCompressionDynamic /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-HttpCompressionStatic /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-Security /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-BasicAuthentication /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-IPSecurity /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-RequestFiltering /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-URLAuthorization /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-ClientCertificateMappingAuthentication /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-DigestAuthentication /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-IISCertificateMappingAuthentication /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-WindowsAuthentication /NoRestart	
	dism /online /Disable-Feature /FeatureName:IIS-ApplicationDevelopment /NoRestart
	dism /online /Disable-Feature /FeatureName:NetFx3 /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-NetFxExtensibility /NoRestart
	dism /online /Disable-Feature /FeatureName:WAS-WindowsActivationService /NoRestart
	dism /online /Disable-Feature /FeatureName:WAS-ProcessModel /NoRestart
	dism /online /Disable-Feature /FeatureName:WAS-NetFxEnvironment /NoRestart
	dism /online /Disable-Feature /FeatureName:WAS-ConfigurationAPI /NoRestart
	dism /online /Disable-Feature /FeatureName:WCF-HTTP-Activation /NoRestart
	dism /online /Disable-Feature /FeatureName:WCF-NonHTTP-Activation /NoRestart
}
else
{	
	dism /online /Disable-Feature /FeatureName:IIS-WebServerRole /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-FTPServer /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-FTPSvc /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-FTPExtensibility /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-WebServerManagementTools /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-ManagementConsole /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-ManagementScriptingTools /NoRestart
	dism /online /Disable-Feature /FeatureName:NetFx4-AdvSrvs /NoRestart
	dism /online /Disable-Feature /FeatureName:NetFx4Extended-ASPNET45 /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-ManagementService /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-WebServer /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-CommonHttpFeatures /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-HttpErrors /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-StaticContent /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-HttpRedirect /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-WebDAV /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-HealthAndDiagnostics /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-CustomLogging /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-HttpLogging /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-RequestMonitor /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-HttpTracing /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-LoggingLibraries /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-Performance /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-HttpCompressionDynamic /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-HttpCompressionStatic /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-Security /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-BasicAuthentication /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-IPSecurity /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-RequestFiltering /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-URLAuthorization /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-RequestFiltering /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-DefaultDocument /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-DirectoryBrowsing /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-ApplicationDevelopment /NoRestart
	dism /online /Disable-Feature /FeatureName:NetFx3 /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-NetFxExtensibility /NoRestart
	dism /online /Disable-Feature /FeatureName:WAS-WindowsActivationService /NoRestart
	dism /online /Disable-Feature /FeatureName:WAS-ProcessModel /NoRestart
	dism /online /Disable-Feature /FeatureName:WAS-NetFxEnvironment /NoRestart
	dism /online /Disable-Feature /FeatureName:WAS-ConfigurationAPI /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-NetFxExtensibility45 /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-ISAPIExtensions /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-ISAPIFilter /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-ASP /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-ASPNET /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-ASPNET45 /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-ApplicationInit /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-CGI /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-ServerSideIncludes /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-WebSockets /NoRestart
	dism /online /Disable-Feature /FeatureName:IIS-HostableWebCore /NoRestart
	dism /online /Disable-Feature /FeatureName:WCF-HTTP-Activation /NoRestart
	dism /online /Disable-Feature /FeatureName:WCF-NonHTTP-Activation /NoRestart
	dism /online /Disable-Feature /FeatureName:WCF-Services45 /NoRestart
	dism /online /Disable-Feature /FeatureName:WCF-HTTP-Activation45 /NoRestart
	dism /online /Disable-Feature /FeatureName:WCF-TCP-PortSharing45 /NoRestart
	dism /online /Disable-Feature /FeatureName:WCF-TCP-Activation45 /NoRestart
	dism /online /Disable-Feature /FeatureName:WCF-Pipe-Activation45 /NoRestart
	dism /online /Disable-Feature /FeatureName:MSMQ-Container /NoRestart
	dism /online /Disable-Feature /FeatureName:MSMQ-Server /NoRestart
	dism /online /Disable-Feature /FeatureName:WCF-MSMQ-Activation45 /NoRestart
}
#netsh advfirewall firewall delete rule name="Wardwiz"