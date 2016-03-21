;(function ($){
  $(function(){
    var name = $('#name')
    var comment = $('#comment')

    var jsonDiv = $('#json')
    var successDiv = $('#success')
    var commentsDiv = $('#comments')

    $('#submit').on('click', function (e){
      e.preventDefault()
      var n = name.val()
      var c = comment.val()

      if(!n || !c){ return clear() }

      var data = {name: n, comment: c}
      clear()

      $.ajax({
        url: '/comment',
        type: 'POST',
        data: data,
        success : function (){
          jsonDiv.text(JSON.stringify(data))
          successDiv.text('success')
        },
        error : function (){
          successDiv.text('unauthorized')
        }
      })
    })

    $('#show').on('click', function (e){
      e.preventDefault()
      clear()
      $.getJSON('/comment', function (comments){
        if(!comments.length){
          return commentsDiv.html('no comments')
        }
        var list = '<ul>'
        comments.forEach(function (comment){
          list += '<li style="margin-top:10px;" data-id=' + comment._id + '> Name: ' + comment.name + ' Comment: ' + comment.comment + '</li>'
        })
        list += '</ul><br>(click a comment to delete)'
        commentsDiv.html(list)
      })
    })

    commentsDiv.on('click', 'li', function (){
      var self = this
      var data = {comment: $(this).attr('data-id')}
      $.ajax({
        url: '/comment',
        type: 'DELETE',
        data: data,
        success : function (){
          $(self).hide()
          successDiv.text('')
        },
        error : function (){
          successDiv.text('unauthorized')
        }
      })
    })

    var clear = function (){
      name.val('')
      comment.val('')
      jsonDiv.text('')
      successDiv.text('')
    }

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

  	$("#findCity").click(function(e){
  		e.preventDefault()
  		var val = cityfield.val()
  		dispcity.text(val)
  		var url = weather + val + '.json'
  		$.ajax({
  			url : url,
        crossDomain: true,
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
  	})

    var username = $('#username')
    var password = $('#password')

    var loginMsg = $('#loginMsg')

    $('#login').on('click', function (e){
  		e.preventDefault()

      var u = username.val()
      var p = password.val()

      if(u && p){
        var data = {username: u, password: p}
        $.post('/login', data, function (){
          loginMsg.text('logged in')
          username.val('')
          password.val('')
          successDiv.text('')
        }).fail(function (){
          loginMsg.text('try lab8:lab8')
          password.val('')
        })
      }
    })

    $('#logout').on('click', function (e){
  		e.preventDefault()
      $.get('/logout', function (){
        loginMsg.text('logged out')
      }).fail(function (){
        loginMsg.text('was not logged in')
      })
    })

    $('#status').on('click', function (e){
  		e.preventDefault()
      $.get('/status', function (data){
        loginMsg.text('status: ' + data)
      })
    })


  })
})(window.jQuery)
