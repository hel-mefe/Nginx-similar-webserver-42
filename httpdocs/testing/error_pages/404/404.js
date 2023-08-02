document.ready(function(){
  var c = document.getElementById('canv');
  var $ = c.getContext('2d');
  c.width = 1000; c.height = 1000; 
  var ms = false;
  var w = 0, h = 0;
  
  var img = new Image();
  // img.src = 'https://s3-us-west-2.amazonaws.com/s.cdpn.io/131045/404.png';
  img.src = "assets/404.png"
  var set = function(){
    var run, _h, _w, dx, a;
    w = img.width;
    h = img.height;
    $.translate(50, 50);
    a = 30;
    dx = 0;
    _w = w + 50;
    _h = h + 50;
    return (run = function() {
      var inc, i, j;
      $.clearRect(-a, -a, _w, _h);
      inc = ms === true ? 0.5 : 0.18;
      for (j=0; j <= h; i = 0 <= h ? ++j : --j) {
        dx = ~~(inc * (Math.random() - 0.5) * a);
        $.drawImage(img, 0, i, w, 1, dx, i, w, 1);
      }
      window.requestAnimationFrame(run);
    })();
  };
  img.onload = function(){
    return set();
  };
  c.addEventListener('mouseover', function(){
    return ms = true;
  }, false);
  c.addEventListener('touchmove',function(e){
    e.preventDefault();
    return ms = true;
  }, false);
  c.addEventListener('mouseout', function(){
    return ms = false;
  }, false);
  c.addEventListener('touchend',function(){
    return ms = false;
  }, false);
}());

