String getHTMLAutomaticGoBack(short millis){
	String htmlPart = "<script language=\"JavaScript\" type=\"text/javascript\">\n";
    htmlPart += "setTimeout(\"window.history.go(-1)\"," + String(millis) + ");\n";
    htmlPart += "</script>\n";
	return htmlPart;
}