$(document).ready(function() {
	var reload = null;
	var datetime = null;
	
	if( $('#loadhere').length > 0 )
	{
		loadContent();
		//reload = setInterval( "loadContent()", 1500 );
	}
		
	if( $('#datetime').length > 0 )
	{
		loadDateTime();
		//datetime = setInterval( "loadDateTime()", 1500 );
	}
	
	$('#mode').click(function() {
		$.ajax({ url: "/json.cgi?mode" });
	});
	
	$('#turbo').click(function() {
		$.ajax({ url: "/json.cgi?turbo" });
	});
	
	$('#repeat').click(function() {
		$.ajax({ url: "/json.cgi?repeat" });
	});
	
	$('#start').click(function() {
		$.ajax({ url: "/json.cgi?%7b%22COMMAND%22:%22CLEAN_START%22%7d" });
	});
	
	$('#pause').click(function() {
		$.ajax({ url: "/json.cgi?%7b%22COMMAND%22:%22PAUSE%22%7d" });
	});
	
	$('#homing').click(function() {
		$.ajax({ url: "/json.cgi?%7b%22COMMAND%22:%22HOMING%22%7d" });
	});
	$('#forward').click(function() {
		$.ajax({ url: "/json.cgi?%7b%22JOY%22:%22FORWARD%22%7d" });
	});
	$('#backward').click(function() {
		$.ajax({ url: "/json.cgi?%7b%22JOY%22:%22BACKWARD%22%7d" });
	});
	$('#left').click(function() {
		$.ajax({ url: "/json.cgi?%7b%22JOY%22:%22LEFT%22%7d" });
	});
	$('#right').click(function() {
		$.ajax({ url: "/json.cgi?%7b%22JOY%22:%22RIGHT%22%7d" });
	});
	$('#forward_left').click(function() {
		$.ajax({ url: "/json.cgi?%7b%22JOY%22:%22FORWARD_LEFT%22%7d" });
	});
	$('#forward_right').click(function() {
		$.ajax({ url: "/json.cgi?%7b%22JOY%22:%22FORWARD_RIGHT%22%7d" });
	});
	$('#backward_left').click(function() {
		$.ajax({ url: "/json.cgi?%7b%22JOY%22:%22BACKWARD_LEFT%22%7d" });
	});
	$('#backward_right').click(function() {
		$.ajax({ url: "/json.cgi?%7b%22JOY%22:%22BACKWARD_RIGHT%22%7d" });
	});
	$('#release').click(function() {
		$.ajax({ url: "/json.cgi?%7b%22JOY%22:%22RELEASE%22%7d" });
	});
	
	$('#setdatetime').click(function() {
		var date = $('#datepicker').val();
		var hour = $('#hourpicker').val();
		var minute = $('#minpicker').val();
		
		if( date.length == 8 && hour.length > 0 && minute.length > 0 )
			$.ajax({ url: "/json.cgi?{%22TIME_SET%22:{%22DATE%22:%22" + date + "%22,%22HOUR%22:%22" + hour + "%22,%22MINUTE%22:%22" + minute + "%22,%22SECOND%22:%2200%22}}" });
		else
			alert( "Wrong format for input values" );
	});
	
	$('#setbotname').click(function() {
		var name = $('#botname').val();
		
		if( name.length > 1 )
			$.ajax({ url: "/json.cgi?{%22NICKNAME%22:{%22SET%22:%22" + name + "%22}}" });
		else
			alert( "Name to short (2-16 characters)" );
	});
	
	$('#fileupload').submit(function(e) {		
		$.ajax({
			url: '/upload.html',
			type: 'POST',
			data: new FormData( this ),
			processData: false,
			contentType: false,
			success:function( htmldata ) {
				var filename;				
				$('#feedback').css( "background-image", 'url(../images/success.png)' );				
				$('#response').html( htmldata );
				
				filename = $('#fname').text();
				console.log( "Found the file: " + filename );
				
				if( $('#delete').length > 0 && filename != "" )
				{
					$('#delete').bind( "click", function() {
						$.ajax({ url: "/remove?" + filename });
						window.location.href = "/sites/service.html";
					});
				}
				
				if( $('#install').length > 0 && filename != "" )
				{
					$('#install').bind( "click", function() {
						$.ajax({
							url: "/activate?" + filename,
							success:function( htmldata ) {
								$('#response').html( htmldata );
					         			
								$('#refresh').bind( "click", function() {
								   location.reload();					  
					         });
								
	                     $('#reboot').bind("click", function() {		
	                        if(confirm("You are about to reboot " + nickname + "'s system. Are you sure?")) {
		                     	$.ajax({ url: "/reboot?",
			                        success:function( htmldata ) {
					         			$('#response').html( htmldata );
					         			setTimeout(function(){
					         				document.location.href='/';
					         			}, 30000);
					               } });	     
	                        }
	                     });
							}
						});
					});
				}
			},
			error:function( htmldata ) {
				$('#feedback').css( "background-image", 'url(../images/error.png)' );
				$('#response').html( htmldata );
			}
		});
		
		e.preventDefault();
	});
});

$(function() {
	$( "#datepicker" ).datepicker({ dateFormat: 'yymmdd' }).val();
});

function loadDateTime() {
	$('#datetime').load( "/sites/date.html", function() {
		window.setTimeout('loadDateTime()', 1000);
	});
}

function loadContent() {
	$('#loadhere').load( "/sites/home.html", function() {
		window.setTimeout('loadContent()', 1000);
	});
}
