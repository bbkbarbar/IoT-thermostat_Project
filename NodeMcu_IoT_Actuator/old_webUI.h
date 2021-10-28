String generateHtmlHeader(){
  String h = "<!DOCTYPE html>";
  h += "<html lang=\"en\">";
  h += "\n\t<head>";
  h += "\n\t\t<meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  h += "\n\t\t<title>" + String(TITLE) + " " + String(VERSION) + "</title>";
  h += "\n\t</head>";
  return h;
}

String generateHtmlBody(){
  String m = "\t<body bgcolor=\"#49B7E8\">";

  m += "\t\t<h4>";
  m += String(SOFTWARE_NAME) + " " + String(VERSION) + " b" + String(BUILDNUM);
  m += "<br>" + String(LOCATION_NAME);
  m += "</h4><br>";
  m += "\n\t<center>\n";

  // FŰTÉS visszajelzése
  if(action == HEATING){
    m += "\t\t<h1>F&Udblac;T&Edot;S</h1>";
  }else{
    m += "\t\t<br>";
  }
  

  m += "\n\t</center>";
  m += "\n<br>ErrorCount: " + String(errorCount);
  m += "\n<br>Elasped time: " + String(elapsedTime);
  m += "\t</body>\r\n";
  m += "</html>";
  
  return m;
}