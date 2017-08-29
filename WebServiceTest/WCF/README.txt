Create Web Service:

1: control panel / programs / programs and features / turn windows features on and off
2: turn on internet information services and internet information services/world wide web services/application development features/ASP.Net 4.6
3: turn on .NET Framework 4.6 Advanced Services/WCF Services/HTTP Activation
4: run visual studio as administrator
5: create new project templates/c#/WCF Service Application - may require microsoft web tools to be installed
6: right click project - publish / set location to c:\inetpub\wwwroot\<project name>\
7: goto http://localhost/<projectname>/<webservicename>.svc

Call Web Service

1: copy System.Runtime.Serialization.dll and System.ServiceModel.dll from C:\Program Files\Unity\Editor\Data\Mono\lib\mono\2.0 to Plugins folder
2: change Api Compatibility Level to .NET 2.0 in player settings
3: update project name in WebServiceTestClientGenerator.bat and run
4: update function calls to match web service calls
5: run