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
      jsonDiv.text(JSON.stringify(data))

      $.post('/comment', data, function (){
        successDiv.text('success')
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
      var data = {comment: $(this).attr('data-id')}
      $.ajax({
        url: '/comment',
        type: 'DELETE',
        data: data
      })
      $(this).hide()
    })

    var clear = function (){
      name.val('')
      comment.val('')
      jsonDiv.text('')
      successDiv.text('')
    }
  })
})(window.jQuery)
