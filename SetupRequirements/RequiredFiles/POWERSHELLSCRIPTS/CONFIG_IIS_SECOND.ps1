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
	#check os is 64bit or 32bit
	
	$checkBits = (get-wmiobject win32_operatingsystem).osarchitecture
	if(($checkBits -eq "64-bit"))
	{
		$frameworkPath =  Get-ChildItem "$env:windir\Microsoft.NET\Framework64" |  where {$_.Attributes -match'Directory'}
		$path = "$env:windir\Microsoft.NET\Framework64\"+ $frameworkPath[$frameworkPath.length - 1].name +"\aspnet_regiis.exe"	 	
	}
	else
	{
		$frameworkPath =  Get-ChildItem "$env:windir\Microsoft.NET\Framework" |  where {$_.Attributes -match'Directory'}
		$path = "$env:windir\Microsoft.NET\Framework\"+ $frameworkPath[$frameworkPath.length - 1].name +"\aspnet_regiis.exe"
	}
	echo $path
	#register ASP.NET applications with Internet Information Services (IIS)
	& $path -i
}	
