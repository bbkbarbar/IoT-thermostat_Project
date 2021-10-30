String getHTML(short heating, int rssi){

	String heatingColor = "";
	if(heating == HEATING){
	  heatingColor = "#f24e21";
	}else{
	  heatingColor = "#737373";
	}

  	String pg = "";
	pg += "<!DOCTYPE html>\n";
	pg += "<html>\n";
	pg += "<head>\n";
	pg += "<title>" + String(MODULE_NAME) + "</title>\n";
	pg += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>\n";
	pg += "<link href='https://fonts.googleapis.com/css?family=Open+Sans:300,400,600' rel='stylesheet'>\n";
	pg += "<style>\n";
	pg += "html { font-family: 'Open Sans', sans-serif; display: block; margin: 0px auto; text-align: center;color: #444444;}\n";
	pg += "body{margin: 0px;} \n";
	
	pg += "h1 {margin: 10px auto 10px;}\n";
	pg += "h2 {margin: 10px auto 0px;}\n";
	pg += "h3 {margin: 0px auto 50px;}\n";

	pg += ".side-by-side{display: table-cell;vertical-align: middle;position: relative;}\n";
	pg += ".text{font-weight: 600;font-size: 19px;width: 200px;}\n";
	pg += ".reading{font-weight: 300;font-size: 50px;padding-right: 25px;}\n";
	pg += ".temperature .reading{color: #26B999;}\n";
	pg += ".temperatureSet .reading{color: #F29C21;}\n";
	pg += ".humidity .reading{color: #3B97D3;}\n";
	pg += ".phasestatus .reading{color: #955BA5;}\n";
	pg += ".heating .reading{color: " + heatingColor + ";}";
	pg += ".superscript{font-size: 17px;font-weight: 600;position: absolute;top: 10px;}\n";
	pg += ".data{padding: 10px;}\n";
	pg += ".container{display: table;margin: 0 auto;}\n";
	pg += ".icon{width:65px}\n";
	pg += "</style>\n";
	pg += "</head>\n";
	pg += "<body>\n";
	pg += "<br>\n";
	pg += "<h2>" + String(SOFTWARE_NAME) + "</h2>";
	pg += "<h1>" + String(MODULE_NAME) + "</h1>";
	pg += "<h3>" + String(VERSION) + "</h3>";
	pg += "<div class='container'>\n";

	pg += "<div class='data heating'>\n";
	pg += "<div class='side-by-side icon'>\n";
	pg += "<svg enable-background='new 0 0 58.422 40.639'height=40.639px id=Layer_1 version=1.1 viewBox='0 0 58.422 40.639'width=58.422px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><g><path d='M58.203,37.754l0.007-0.004L42.09,9.935l-0.001,0.001c-0.356-0.543-0.969-0.902-1.667-0.902\n";
	pg += "c-0.655,0-1.231,0.32-1.595,0.808l-0.011-0.007l-0.039,0.067c-0.021,0.03-0.035,0.063-0.054,0.094L22.78,37.692l0.008,0.004\n";
	pg += "c-0.149,0.28-0.242,0.594-0.242,0.934c0,1.102,0.894,1.995,1.994,1.995v0.015h31.888c1.101,0,1.994-0.893,1.994-1.994\n";
	pg += "C58.422,38.323,58.339,38.024,58.203,37.754z'fill=#955BA5 /><path d='M19.704,38.674l-0.013-0.004l13.544-23.522L25.13,1.156l-0.002,0.001C24.671,0.459,23.885,0,22.985,0\n";
	pg += "c-0.84,0-1.582,0.41-2.051,1.038l-0.016-0.01L20.87,1.114c-0.025,0.039-0.046,0.082-0.068,0.124L0.299,36.851l0.013,0.004\n";
	pg += "C0.117,37.215,0,37.62,0,38.059c0,1.412,1.147,2.565,2.565,2.565v0.015h16.989c-0.091-0.256-0.149-0.526-0.149-0.813\n";
	pg += "C19.405,39.407,19.518,39.019,19.704,38.674z'fill=#955BA5 /></g></svg>\n";
	pg += "</div>\n";
	pg += "<div class='side-by-side text'>Heating</div>\n";
	pg += "<div class='side-by-side reading'>\n";
	if(heating == HEATING){
		pg += "<b>Active</b>\n";
	}else{
    if(heating == NOTHING){
      pg += "Passive\n";		
    }else{
      pg += "Undefined\n";
    }
	}
	pg += "</div>\n";
	pg += "</div>\n";
	pg += "</div>\n";
	pg += "<h2>RSSI: " + String(rssi) + "</h2>\n";
  #ifdef OVERRIDE_WITH_FACTORY_CONTROL
    pg +="<h1>Factory input used!</h1>\n";
  #endif
	#ifdef USE_MOCK
		pg +="<h1>MOCK USED</h1>\n";
		#ifdef THERMOSTAT_MOCK_IP
			pg +="<h2>Mock thermostat ip:</h2>\n";
			pg +="<h2>" + String(THERMOSTAT_MOCK_IP) + "</h2>\n";
		#else
			pg +="<h2>Mock thermostat ip UNDEFINED!</h2>\n";
		#endif
	#endif
	
	pg += "</body></html>\n";

	return pg;
}
