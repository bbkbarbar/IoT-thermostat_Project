String getHTML(short heating, int rssi, short mode){

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
	pg += ".mode .reading{color: #955BA5;}\n";
  pg += ".tech .reading{color: #8a8a8a;}\n";
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

  // MODE
  pg += "<div class='data mode'>\n";
  pg += "<div class='side-by-side icon'>\n";
  pg += "<svg enable-background='new 0 0 40.542 40.541'height=40.541px id=Layer_1 version=1.1 viewBox='0 0 40.542 40.541'width=40.542px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><g><path d='M34.313,20.271c0-0.552,0.447-1,1-1h5.178c-0.236-4.841-2.163-9.228-5.214-12.593l-3.425,3.424\n";
  pg += "c-0.195,0.195-0.451,0.293-0.707,0.293s-0.512-0.098-0.707-0.293c-0.391-0.391-0.391-1.023,0-1.414l3.425-3.424\n";
  pg += "c-3.375-3.059-7.776-4.987-12.634-5.215c0.015,0.067,0.041,0.13,0.041,0.202v4.687c0,0.552-0.447,1-1,1s-1-0.448-1-1V0.25\n";
  pg += "c0-0.071,0.026-0.134,0.041-0.202C14.39,0.279,9.936,2.256,6.544,5.385l3.576,3.577c0.391,0.391,0.391,1.024,0,1.414\n";
  pg += "c-0.195,0.195-0.451,0.293-0.707,0.293s-0.512-0.098-0.707-0.293L5.142,6.812c-2.98,3.348-4.858,7.682-5.092,12.459h4.804\n";
  pg += "c0.552,0,1,0.448,1,1s-0.448,1-1,1H0.05c0.525,10.728,9.362,19.271,20.22,19.271c10.857,0,19.696-8.543,20.22-19.271h-5.178\n";
  pg += "C34.76,21.271,34.313,20.823,34.313,20.271z M23.084,22.037c-0.559,1.561-2.274,2.372-3.833,1.814\n";
  pg += "c-1.561-0.557-2.373-2.272-1.815-3.833c0.372-1.041,1.263-1.737,2.277-1.928L25.2,7.202L22.497,19.05\n";
  pg += "C23.196,19.843,23.464,20.973,23.084,22.037z'fill=#955BA5 /></g></svg>\n";
  pg += "</div>\n";
  pg += "<div class='side-by-side text'>Mode</div>\n";
  pg += "<div class='side-by-side reading'>\n";
  if(mode == MODE_FORWARDER){
    pg += "Factory";
  }else{
    pg += "Custom";
  }
  pg += "</div>\n";
  pg += "</div>\n";

  // HEATING
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

  // RSSI
  pg += "<div class='data tech'>\n";
  pg += "<div class='side-by-side icon'>\n";
  pg += "<svg enable-background='new 0 0 40.542 40.541'height=40.541px id=Layer_1 version=1.1 viewBox='0 0 40.542 40.541'width=40.542px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><g><path d='M34.313,20.271c0-0.552,0.447-1,1-1h5.178c-0.236-4.841-2.163-9.228-5.214-12.593l-3.425,3.424\n";
  pg += "c-0.195,0.195-0.451,0.293-0.707,0.293s-0.512-0.098-0.707-0.293c-0.391-0.391-0.391-1.023,0-1.414l3.425-3.424\n";
  pg += "c-3.375-3.059-7.776-4.987-12.634-5.215c0.015,0.067,0.041,0.13,0.041,0.202v4.687c0,0.552-0.447,1-1,1s-1-0.448-1-1V0.25\n";
  pg += "c0-0.071,0.026-0.134,0.041-0.202C14.39,0.279,9.936,2.256,6.544,5.385l3.576,3.577c0.391,0.391,0.391,1.024,0,1.414\n";
  pg += "c-0.195,0.195-0.451,0.293-0.707,0.293s-0.512-0.098-0.707-0.293L5.142,6.812c-2.98,3.348-4.858,7.682-5.092,12.459h4.804\n";
  pg += "c0.552,0,1,0.448,1,1s-0.448,1-1,1H0.05c0.525,10.728,9.362,19.271,20.22,19.271c10.857,0,19.696-8.543,20.22-19.271h-5.178\n";
  pg += "C34.76,21.271,34.313,20.823,34.313,20.271z M23.084,22.037c-0.559,1.561-2.274,2.372-3.833,1.814\n";
  pg += "c-1.561-0.557-2.373-2.272-1.815-3.833c0.372-1.041,1.263-1.737,2.277-1.928L25.2,7.202L22.497,19.05\n";
  pg += "C23.196,19.843,23.464,20.973,23.084,22.037z'fill=#8a8a8a /></g></svg>\n";
  pg += "</div>\n";
  pg += "<div class='side-by-side text'>RSSI</div>\n";
  pg += "<div class='side-by-side reading'>\n";
  pg += String(rssi);
  pg += "</div>\n";
  pg += "</div>\n";

  pg += "</div>\n";
  
 
	//pg += "<h2>RSSI: " + String(rssi) + "</h2>\n";
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
