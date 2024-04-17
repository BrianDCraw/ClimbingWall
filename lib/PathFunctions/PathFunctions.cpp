#include <PathFunctions.h>
#include <Arduino.h>
    
        String PathFunctions::GetCleanPath(String path)
        {
            if (path.endsWith("/")) {
              path += "index.html";
            }
            else if (path.endsWith(".src")) {
              path = path.substring(0, path.lastIndexOf("."));
            }
            return path;
        }
        
        String PathFunctions::GetFileType(String path)
        {
            String dataType = "text/plain";
            if (path.endsWith(".html")) {
            dataType = "text/html";
            } else if (path.endsWith(".css")) {
            dataType = "text/css";
            } else if (path.endsWith(".js")) {
             dataType = "text/javascript";
            } else if (path.endsWith(".png")) {
             dataType = "image/png";
            } else if (path.endsWith(".gif")) {
             dataType = "image/gif";
            } else if (path.endsWith(".jpg")) {
             dataType = "image/jpeg";
            } else if (path.endsWith(".ico")) {
             dataType = "image/x-icon";
            } else if (path.endsWith(".xml")) {
             dataType = "text/xml";
            } else if (path.endsWith(".pdf")) {
             dataType = "application/pdf";
            } else if (path.endsWith(".zip")) {
             dataType = "application/zip";
            }

            return dataType;
        }