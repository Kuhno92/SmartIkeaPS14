const char MAIN_page[] PROGMEM = R"=====(
<meta name="viewport" content="width=device-width, initial-scale=1" /> 
<link rel="stylesheet" href="http://code.jquery.com/mobile/1.4.5/jquery.mobile-1.4.5.min.css" />
<script src="http://code.jquery.com/jquery-1.11.1.min.js"></script>
<script src="http://code.jquery.com/mobile/1.4.5/jquery.mobile-1.4.5.min.js"></script>
<script src="https://cdnjs.cloudflare.com/ajax/libs/jscolor/2.0.4/jscolor.min.js" type="text/javascript"></script>
<!-- Init Page -->
<script type="text/javascript">
    var potiMax = 360;
    var potiMin = 220;
    var init; //position = 720, light = 1
    var xhr = new XMLHttpRequest();
    xhr.open('post', '/initPage');
    xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
    xhr.send();
    xhr.onreadystatechange = function() 
      { 
          // If the request completed, close the extension popup
          if (xhr.readyState == 4){
            if (xhr.status == 200){
              init = JSON.parse(xhr.responseText);
              console.log("Initializing with: ");
              console.log(init);
              setTimeout(function(){
                $("#deathstarSlider").val(init.position).slider("refresh"); //(potiMax+potiMin-init.position);
              },200);
              if(init.position < (potiMax+potiMin)/2) {
                  $("#deathstar img.close").removeClass("invisible");
                } else if(init.position >= (potiMax+potiMin)/2){
                  $("#deathstar img.open").removeClass("invisible");
              }
              if(init.light > 0) {
                $("#light-bulb").removeClass("invisible");
              } else {
                $("#light-bulb2").removeClass("invisible");
              }
            }
          }
      };
  </script>
<body>
<div data-role="page" data-theme="b">
    <div data-role="header">
      <h1>Ikea PS 14 - Yeelight</h1>
    </div>

    <div id="main" data-role="main" class="ui-content">
  <style>
    #deathstar img.invisible {opacity:0;}
    #lightbulb div.invisible {opacity:0;}
    #cf_onclick {cursor:pointer;}
    /*body {background: #466368; background: linear-gradient(to right bottom, #648880, #293f50);}*/

    /*First rule hides the text box, the second one makes slider full width*/
    div.ui-slider-slider input.ui-input-text {
      display: none;
    }
    div.ui-slider-slider div.ui-slider-track {
        margin: 0 15px 0 15px !important;
    }

  </style>

    <!-- Deathstar -->
  <div id="deathstar" class="deathstar" style="position:relative; height:60vw; width:60vw; max-width:300px; max-height:300px; margin:0 auto;">
    <figure id="topimg">
      <img id="open" class="open invisible" src="https://image.ibb.co/jddKfQ/opened.png" style="position:absolute; left:0; height:100%; width:100%; -webkit-transition: opacity 1s ease-in-out; -moz-transition: opacity 1s ease-in-out;  -o-transition: opacity 1s ease-in-out; transition: opacity 1s ease-in-out;"onclick="deathstarLogic(this);"/>
      <img id="close" class="close invisible" src="https://image.ibb.co/c2oKfQ/closed.png" style=" position:absolute; left:0; height:100%; width:100%; -webkit-transition: opacity 1s ease-in-out; -moz-transition: opacity 1s ease-in-out;  -o-transition: opacity 1s ease-in-out; transition: opacity 1s ease-in-out;" onclick="deathstarLogic(this);"/>
    </figure>
  </div>


  <!-- Slider -->
  <style>input[type=range]::-webkit-slider-thumb {
      -webkit-appearance: none;
      border: none;
      height: 16px;
      width: 16px;
      border-radius: 50%;
      background: goldenrod;
      margin-top: -4px;
    }
  </style>
  <div id="deathstarSliderwrapper" class="ui-slider-slider" style="text-align: center; color:white; text-shadow: black 0.1em 0.1em 0.2em;""> 
    <label style="font-size: 130%;">Position:</label>
    <input id="deathstarSlider" type="range" min="220" max="360" class="ui-hidden-accessible" data-theme="a" data-track-theme="b" />
  </div>

  <!-- Lightbulb -->
  <div id="lightbulb" class="lightbulb" style="position:relative; height:150px; width:150px; margin:0 auto;">
  <figure id="topimg">
    <div id="light-bulb" class="off ui-draggable invisible" style="position:absolute; left:0; width: 150px; height: 150px; background: url(https://lh4.googleusercontent.com/-katLGTSCm2Q/UJC0_N7XCrI/AAAAAAAABq0/6GxNfNW-Ra4/s300/lightbulb.png) no-repeat 0 0; -webkit-transition: opacity 0.5s ease-in-out; -moz-transition: opacity 0.5s ease-in-out;  -o-transition: opacity 0.5s ease-in-out; transition: opacity 0.5s ease-in-out;" onclick="lightbulbLogic(this);"></div>
    <div id="light-bulb2" class="off ui-draggable invisible" style=" position: absolute; width: 150px; height: 150px; left:0; background: url(https://lh4.googleusercontent.com/-katLGTSCm2Q/UJC0_N7XCrI/AAAAAAAABq0/6GxNfNW-Ra4/s300/lightbulb.png) no-repeat -150px 0; -webkit-transition: opacity 0.5s ease-in-out; -moz-transition: opacity 0.5s ease-in-out;  -o-transition: opacity 0.5s ease-in-out; transition: opacity 0.5s ease-in-out;" onclick="lightbulbLogic(this);"></div>
  </figure>
  </div>

  <div id="brightnesSliderrapper" class="ui-slider-slider" style="text-align: center; color:white; text-shadow: black 0.1em 0.1em 0.2em; "> 
    <label style="font-size: 130%;">Brightness:</label>
    <input id="brightnesSlider" type="range" min="1" max="100" value="100" data-theme="a" data-track-theme="b" class="ui-hidden-accessible" />
  </div>

   <div id="colorchangerwrapper"  style="display: flex; justify-content: center; left:50%; text-align: center;"> 
   <style type="text/css">
    .jscolorbuttonstyle {
      width: 305px !important;
  }
   </style>
    <input id="btncolor" data-wrapper-class="jscolorbuttonstyle"  class="jscolor {mode:'HS', position:'top'}" onchange="colorChangeLogic(this.jscolor)" value="Color" type="button" style=" margin:0 auto; top:0%; text-indent: -299em;">
  </div>

  <style type="text/css">
    .ui-collapsible-heading .ui-btn  {
            text-align: center;
  }
  </style>
  <div data-role="collapsible"><h4>Modes</h4>
  <div data-role="collapsible"><h4>Party</h4>
     <input class="btnmode"  value="Party(very fast)" type="button">
     <input class="btnmode"  value="Party(fast)" type="button">
     <input class="btnmode"  value="Party(medium)" type="button">
     <input class="btnmode"  value="Party(slow)" type="button">
     <input class="btnmode"  value="Party(very slow)" type="button">
  </div>
  <input class="btnmode"  value="Fire" type="button">
  <input class="btnmode"  value="Sunset" type="button">
  <input class="btnmode"  value="Sunrise" type="button">
  <input class="btnmode"  value="Night Mode" type="button">
  <input class="discover" value="Discover(Dev.)" type="button">
  </div>
  <div data-role="collapsible"><h4>Development</h4>
    <div id="console-output" style="overflow-y: auto; max-height: 20vh;">
      Console-output:
    </div>
    <button data-mini=true id="console-refresh-btn" >Refresh</button>
  </div>
  <div data-role="footer" style="position:relative; bottom: 0; left: 0; bottom:0; left:0; background: #466368); color:white; text-shadow: black 0.1em 0.1em 0.2em;">
    <h3 id="footer">Copyright Nico Kuhn 2017 - BootUp Time: </h3>
  </div>
  </div>
  
  <script type="text/javascript">
    var openclose = "";  
    function deathstarLogic (e) {
      $("#deathstar img.open").toggleClass("invisible");
      $("#deathstar img.close").toggleClass("invisible");
      openclose = "close";
      //if closing image contains invisible class
      if(e.className.indexOf("invisible") !== -1){
        openclose="open"
      }
      (openclose == "open") ? $("#deathstarSlider").val(potiMax).slider("refresh") : $("#deathstarSlider").val(potiMin).slider("refresh");
      var xhr = new XMLHttpRequest();
      xhr.open('post', '/openCloseDeathstar');
      xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
      xhr.send('openclose='+openclose);
    };
  </script>
  <script type="text/javascript">
    $(document).on('pageinit', function() {
      $( "#btncolor" ).bind( "click", function(event, ui) {
        document.getElementById("main").setAttribute("style","touch-action: none;");
    });
    $( "#main" ).bind( "click", function(e) {
        if(e.target.id != "btncolor"){
          document.getElementById("main").style.removeProperty('touch-action');
        }
    });
    $( "#console-refresh-btn" ).bind( "click", function(event, ui) {
      console.log("Refreshing...")
      var res; 
      var xhr = new XMLHttpRequest();
      xhr.open('post', '/console');
      xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
      xhr.send();
      xhr.onreadystatechange = function() 
        { 
          // If the request completed, close the extension popup
          if (xhr.readyState == 4){
            if (xhr.status == 200){
              console.log(xhr.responseText)
              init = JSON.parse(xhr.responseText);
              console.log("Console data received!");
              document.getElementById("console-output").innerHTML = init.text.replace(/(\r\n|\n|\r)/gm, "");
            }
          }
        };
    });
    $( ".btnmode" ).bind( "click", function(e) {
      var mode = e.target.value;
      console.log("Setting Mode to: " + mode);
      var xhr = new XMLHttpRequest();
      xhr.open('post', '/setMode');
      xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
      xhr.send('value='+ mode);
      });
       $( ".discover" ).bind( "click", function(e) {
        var mode = e.target.value;
      console.log("Setting Mode to: " + mode);
      var xhr = new XMLHttpRequest();
      xhr.open('post', '/discover');
      xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
      xhr.send('value='+ mode);
    });
    // Deathstar Logic -->
    $("#deathstarSlider").on( 'slidestop', function( event ) {
      var e = document.getElementById("deathstarSlider").value;
      if(e > (potiMax+potiMin)/2) {
        $("#deathstar img.open").removeClass("invisible");
        $("#deathstar img.close").addClass("invisible");
      } else if(e < (potiMax+potiMin)/2){
        $("#deathstar img.open").addClass("invisible");
        $("#deathstar img.close").removeClass("invisible");
      }
      var setValue = e; //potiMax + potiMin - e;
      console.log("Setting Deathstar to: " + setValue);
      var xhr = new XMLHttpRequest();
      xhr.open('post', '/setDeathstarPosition');
      xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
      xhr.send('value='+ setValue);
    });
    
    //Lightbulb Logic -->
    $("#brightnesSlider").on( 'slidestop', function( event ) {
      var e = document.getElementById("brightnesSlider").value;
      var setValue = e;
      console.log("Setting Brighness to: " + setValue);
      var xhr = new XMLHttpRequest();
      xhr.open('post', '/setBrightness');
      xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
      xhr.send('value='+ setValue);
    });

  });
      var onoff = ""; 
    function lightbulbLogic (e) {
      $("#light-bulb").toggleClass("invisible");
      $("#light-bulb2").toggleClass("invisible");
      onoff = "off";
      //if closing image contains invisible class
      if(e.className.indexOf("invisible") !== -1){
        onoff="on"
      }
      var xhr = new XMLHttpRequest();
      xhr.open('post', '/lightbulb');
      xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
      xhr.send('onoff='+onoff);
    };
    function colorChangeLogic(jscolor) {
      var setValue = jscolor.toHEXString();
      console.log("Setting Color to: " + setValue);
      var xhr = new XMLHttpRequest();
      xhr.open('post', '/setColor');
      xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
      xhr.send('value='+ setValue);
    }
  </script>

  <script type="text/javascript">
    var div = document.getElementById('footer');
    var xmlHttp = new XMLHttpRequest();
    xmlHttp.open( "GET", "/uptime", false ); // false for synchronous request
    xmlHttp.send( null );
  div.innerHTML = div.innerHTML + xmlHttp.responseText;
  </script>
  </div>
</body>
)=====";
