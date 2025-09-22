#enable iis feature 

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
	dism /online /Enable-Feature /FeatureName:IIS-WebServerRole /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-FTPServer /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-FTPSvc /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-FTPExtensibility /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-WebServerManagementTools /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-ManagementConsole /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-ManagementScriptingTools /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-ManagementService /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-WebServer /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-CommonHttpFeatures /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-HttpErrors /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-StaticContent /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-HttpRedirect /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-WebDAV /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-HealthAndDiagnostics /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-CustomLogging /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-HttpLogging /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-RequestMonitor /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-HttpTracing /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-LoggingLibraries /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-ODBCLogging /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-Performance /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-HttpCompressionDynamic /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-HttpCompressionStatic /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-Security /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-BasicAuthentication /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-IPSecurity /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-RequestFiltering /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-URLAuthorization /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-ClientCertificateMappingAuthentication /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-DigestAuthentication /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-IISCertificateMappingAuthentication /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-WindowsAuthentication /NoRestart	
	dism /online /Enable-Feature /FeatureName:IIS-ApplicationDevelopment /NoRestart
	dism /online /Enable-Feature /FeatureName:NetFx3 /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-NetFxExtensibility /NoRestart
	dism /online /Enable-Feature /FeatureName:WAS-WindowsActivationService /NoRestart
	dism /online /Enable-Feature /FeatureName:WAS-ProcessModel /NoRestart
	dism /online /Enable-Feature /FeatureName:WAS-NetFxEnvironment /NoRestart
	dism /online /Enable-Feature /FeatureName:WAS-ConfigurationAPI /NoRestart
	dism /online /Enable-Feature /FeatureName:WCF-HTTP-Activation /NoRestart
	dism /online /Enable-Feature /FeatureName:WCF-NonHTTP-Activation /NoRestart
}
else
{	
	dism /online /Enable-Feature /FeatureName:IIS-WebServerRole /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-FTPServer /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-FTPSvc /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-FTPExtensibility /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-WebServerManagementTools /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-ManagementConsole /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-ManagementScriptingTools /NoRestart
	dism /online /Enable-Feature /FeatureName:NetFx4-AdvSrvs /NoRestart
	dism /online /Enable-Feature /FeatureName:NetFx4Extended-ASPNET45 /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-ManagementService /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-WebServer /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-CommonHttpFeatures /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-HttpErrors /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-StaticContent /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-HttpRedirect /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-WebDAV /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-HealthAndDiagnostics /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-CustomLogging /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-HttpLogging /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-RequestMonitor /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-HttpTracing /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-LoggingLibraries /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-Performance /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-HttpCompressionDynamic /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-HttpCompressionStatic /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-Security /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-BasicAuthentication /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-IPSecurity /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-RequestFiltering /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-URLAuthorization /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-RequestFiltering /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-DefaultDocument /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-DirectoryBrowsing /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-ApplicationDevelopment /NoRestart
	dism /online /Enable-Feature /FeatureName:NetFx3 /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-NetFxExtensibility /NoRestart
	dism /online /Enable-Feature /FeatureName:WAS-WindowsActivationService /NoRestart
	dism /online /Enable-Feature /FeatureName:WAS-ProcessModel /NoRestart
	dism /online /Enable-Feature /FeatureName:WAS-NetFxEnvironment /NoRestart
	dism /online /Enable-Feature /FeatureName:WAS-ConfigurationAPI /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-NetFxExtensibility45 /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-ISAPIExtensions /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-ISAPIFilter /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-ASP /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-ASPNET /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-ASPNET45 /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-ApplicationInit /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-CGI /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-ServerSideIncludes /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-WebSockets /NoRestart
	dism /online /Enable-Feature /FeatureName:IIS-HostableWebCore /NoRestart
	dism /online /Enable-Feature /FeatureName:WCF-HTTP-Activation /NoRestart
	dism /online /Enable-Feature /FeatureName:WCF-NonHTTP-Activation /NoRestart
	dism /online /Enable-Feature /FeatureName:WCF-Services45 /NoRestart
	dism /online /Enable-Feature /FeatureName:WCF-HTTP-Activation45 /NoRestart
	dism /online /Enable-Feature /FeatureName:WCF-TCP-PortSharing45 /NoRestart
	dism /online /Enable-Feature /FeatureName:WCF-TCP-Activation45 /NoRestart
	dism /online /Enable-Feature /FeatureName:WCF-Pipe-Activation45 /NoRestart
	dism /online /Enable-Feature /FeatureName:MSMQ-Container /NoRestart
	dism /online /Enable-Feature /FeatureName:MSMQ-Server /NoRestart
	dism /online /Enable-Feature /FeatureName:WCF-MSMQ-Activation45 /NoRestart
}
if ((Test-Path "HKLM:\SOFTWARE\Microsoft\InetStp") -or (Test-Path "$env:systemroot\System32\inetsrv\inetmgr.exe") -or (Test-Path "$env:systemroot\SysWow64\inetsrv\inetmgr.exe"))
{
	#iis add website
	Import-Module WebAdministration

	#obtain list of application pools
	#$appPools = Get-ChildItem -Path IIS:\AppPools -Name 

	#ipaddress obtained
	$IPAddrHolder = test-connection $env:computername -count 1 | select IPV4Address
	$temp = "$IPAddrHolder".split("=,}")
			
	#host a website 
	New-Website -Name 'Wardwiz' -Port $args[0] -IPAddress $temp[1] -PhysicalPath 'C:\Program Files\WardWiz\WardWizDev\WardWizEPS' -ApplicationPool 'DefaultAppPool'		
}
