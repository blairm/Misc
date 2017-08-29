Create Web Service:

1: control panel / programs / programs and features / turn windows features on and off
2: turn on internet information services and internet information services/world wide web services/application development features/ASP.Net 4.6
3: run visual studio as administrator
4: create new project templates/c#/ASP.Net web application - may require microsoft web tools to be installed
5: right click project - add new item / web service
6: right click project - publish / set location to C:\inetpub\wwwroot\<project name>\
7: goto http://localhost/<projectname>/<webservicename>.asmx

Call Web Service

1: copy System.Web.dll and System.Web.Services.dll from C:\Program Files\Unity\Editor\Data\Mono\lib\mono\2.0 to Plugins folder
2: change Api Compatibility Level to .NET 2.0 in player settings
3: update project name in WebServiceTestGenerator.bat and run
4: update function calls to match web service calls
5: run