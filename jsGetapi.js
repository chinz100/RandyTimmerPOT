function myFunction() {
    setInterval(function(){ var xhr = new XMLHttpRequest();
        xhr.withCredentials = true;
        xhr.addEventListener('readystatechange', function() {
          if(this.readyState === 4) {
            console.log(this.responseText);
          }
        });
        xhr.open('GET', '/apitimmer');
        xhr.send(); }, 1000);
  }