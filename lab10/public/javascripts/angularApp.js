angular.module('clusterApp', [])
  .controller('MainCtrl', ['$scope', '$http', function($scope, $http){
    $scope.cluster = []

    $scope.getMyPIDs = function() {
      for(i = 0; i < 100; i++) {
      	$http.get('/pid').success(function(data){
      	  $scope.cluster.push(data);
      	});
      }
    }
  }
]);
