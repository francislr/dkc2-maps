$exePath = "$($env:USERPROFILE)\vc_mbcsmfc.exe"
(New-Object Net.WebClient).DownloadFile('http://download.microsoft.com/download/0/2/3/02389126-40A7-46FD-9D83-802454852703/vc_mbcsmfc.exe', $exePath)
cmd /c start /wait $exePath /quiet

