$(document).ready(function() {
	var cityfield = $("#cityfield")
	var dispcity = $("#dispcity")
	var key = 'b709624ac76567ff'
	var weather = 'http://api.wunderground.com/api/'+key+'/geolookup/conditions/q/UT/'

	cityfield.keyup(function() {
		var url = "/cities?q=" + cityfield.val()
		$.getJSON(url, function(data) {
			var everything = '<ul>'
			$.each(data, function (i,item) {
				everything += "<li> " + data[i].city
			})
			$("#txtHint").html(everything + '</ul>')
		})
	})

	$("#button").click(function(e){
		e.preventDefault()
		var val = cityfield.val()
		dispcity.text(val)
		var url = weather + val + '.json'
		$.ajax({
			url : url,
			dataType : "jsonp",
			success : function(parsed_json) {
				var location = parsed_json['location']['city']
				var temp_string = parsed_json['current_observation']['temperature_string']
				var current_weather = parsed_json['current_observation']['weather']
				var everything = "<ul>"
					everything += "<li>Location: "+location
					everything += "<li>Temperature: "+temp_string
					everything += "<li>Weather: "+current_weather
					everything += "</ul>"
				$("#weather").html(everything)
			}
		})

		$.ajax({
			url : '/joke',
			success : function(joke) {
				$("#joke").html(joke)
			}
		})

	})
})
