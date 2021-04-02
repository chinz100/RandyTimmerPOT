function function2(){let hourjava=document.getElementById('Hourdropdown').value;let minjava=document.getElementById('Hourdropdown').value;var data=`0=${hourjava}&1=${minjava}`;var xhr=new XMLHttpRequest();xhr.withCredentials=!0;xhr.addEventListener('readystatechange',function(){if(this.readyState===4){console.log(this.responseText)}});xhr.open('POST','/set');xhr.setRequestHeader('Content-Type','application/x-www-form-urlencoded');xhr.send(data)};window.onload=function(){for(var e=document.getElementById('Hourdropdown').getElementsByTagName('select')[0],o=0;o<=23;o++){(t=new Option).text=o+' HH',t.value=o,e.options[o]=t}var n=document.getElementById('Mindropdown');for(o=0;o<=59;o++){var t;(t=new Option).text=o+' MM',t.value=o,n.options[o]=t}}



function function2(){
    var xhr = new XMLHttpRequest();
    xhr.withCredentials = true;
    
    xhr.addEventListener('readystatechange', function() {
      if(this.readyState === 4) {
        console.log(this.responseText);
      }
    });
    
    xhr.open('GET', 'http://192.168.2.36/apipause');
    
    xhr.send();
}

function function2(){var xhr=new XMLHttpRequest();xhr.withCredentials=!0;xhr.addEventListener('readystatechange',function(){if(this.readyState===4){console.log(this.responseText)}});xhr.open('GET','http://192.168.2.36/apipause');xhr.send()}