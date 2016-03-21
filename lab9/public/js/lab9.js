;(function (){
var app = angular.module('Comments', [])

app.controller('Main', ['$scope', '$http', function($scope, $http){
  $scope.order = 'upVotes'
  $scope.sort = true
  $scope.comments = []

  $http.get('/comments').success(function (data){
    $scope.comments = data
  })

  $scope.saveComment = function (){
    var c = $scope.newComment
    if (c){
      $scope.newComment = ''
      $http.post('/save', { comment: c })
      $scope.comments.push({ comment: c, upVotes: 0, downVotes: 0 })
    }
  }

  $scope.upVote = function (c){
    $http.post('/upVote', { id: c._id })
    c.upVotes++
  }

  $scope.downVote = function (c){
    $http.post('/downVote', { id: c._id })
    c.downVotes++
  }

  $scope.delete = function (c){
    $http.post('/delete', { id: c._id }).success(function (data){
      $scope.comments = data
    })
  }
}])
})()
