const char index_html[] = R"rawliteral(
<!DOCTYPE html><html><head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
<title>SmartButton</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:-apple-system,sans-serif;background:#1a1a2e;color:#eee;padding:16px;min-height:100vh}
h1{text-align:center;font-size:1.3em;margin-bottom:16px;color:#00d4ff}
.card{background:#16213e;border-radius:12px;padding:16px;margin-bottom:14px}
.card h2{font-size:1em;margin-bottom:12px;color:#00d4ff;border-bottom:1px solid #0f3460;padding-bottom:8px}
label{display:block;font-size:.85em;color:#aaa;margin-bottom:4px}
input,select{width:100%;padding:10px;border:1px solid #0f3460;border-radius:8px;background:#0f3460;color:#eee;font-size:.95em;margin-bottom:10px;-webkit-appearance:none}
input:focus,select:focus{outline:none;border-color:#00d4ff}
select{cursor:pointer}
.pass-wrap{position:relative}
.pass-wrap input{padding-right:44px}
.pass-toggle{position:absolute;right:8px;top:6px;background:none;border:none;color:#aaa;font-size:1.2em;cursor:pointer;padding:4px;width:auto}
.wifi-list{max-height:200px;overflow-y:auto;margin-bottom:10px}
.wifi-item{display:flex;justify-content:space-between;align-items:center;padding:10px;border-radius:8px;cursor:pointer;margin-bottom:4px;background:#0f3460}
.wifi-item:hover,.wifi-item.sel{background:#1a3a6e;border:1px solid #00d4ff}
.wifi-item .name{font-size:.9em;flex:1;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
.wifi-item .rssi{font-size:.75em;color:#aaa;margin-left:8px;white-space:nowrap}
.wifi-item .lock{font-size:.75em;margin-left:4px}
button,.btn{width:100%;padding:12px;border:none;border-radius:8px;font-size:.95em;font-weight:bold;cursor:pointer;color:#fff;margin-top:4px}
.btn-scan{background:#0f3460;color:#00d4ff}
.btn-save{background:#00d4ff;color:#1a1a2e}
.btn-save:active{background:#00a8cc}
.btn-test{background:#0f3460;color:#e8a838;margin-top:6px}
.btn-admin{background:#e74c3c;color:#fff}
.btn-login{background:#00d4ff;color:#1a1a2e;margin-top:10px}
.msg{text-align:center;padding:8px;border-radius:8px;margin-top:8px;font-size:.85em;display:none}
.msg.ok{display:block;background:#0a3d2a;color:#4ade80}
.msg.err{display:block;background:#3d0a0a;color:#f87171}
.payload-row{display:none}
.payload-row.show{display:block}
.tabs{display:flex;gap:6px;margin-bottom:10px}
.tab{flex:1;padding:8px;text-align:center;border-radius:8px;background:#0f3460;color:#aaa;cursor:pointer;font-size:.85em}
.tab.active{background:#1a3a6e;color:#00d4ff;border:1px solid #00d4ff}
.row2{display:flex;gap:8px}
.row2>div{flex:1}
.hidden{display:none}
.login-box{max-width:320px;margin:60px auto 0}
.login-box h1{margin-bottom:24px}
</style>
</head><body>

<div id="login" class="login-box">
<h1>🔐 SmartButton</h1>
<div class="card">
<label>Usuario</label>
<input id="luser" placeholder="admin" value="admin">
<label>Contraseña</label>
<div class="pass-wrap">
<input id="lpass" type="password" placeholder="Contraseña">
<button class="pass-toggle" type="button" onclick="togVis('lpass')">👁</button>
</div>
<button class="btn btn-login" onclick="doLogin()">Entrar</button>
<div id="lmsg" class="msg"></div>
</div>
</div>

<div id="app" class="hidden">
<h1>SmartButton Config</h1>

<div class="card">
<h2>WiFi</h2>
<button class="btn btn-scan" onclick="scanWifi()">Buscar redes WiFi</button>
<div id="wlist" class="wifi-list"></div>
<label>SSID</label>
<input id="ssid" placeholder="Nombre de la red">
<label>Password</label>
<div class="pass-wrap">
<input id="pass" type="password" placeholder="Contraseña WiFi">
<button class="pass-toggle" type="button" onclick="togVis('pass')">👁</button>
</div>
<button class="btn btn-save" onclick="saveWifi()">Guardar WiFi</button>
<div id="wmsg" class="msg"></div>
</div>

<div class="card">
<h2>Botones</h2>
<div class="tabs">
<div class="tab active" onclick="switchTab(1,this)">Boton 1</div>
<div class="tab" onclick="switchTab(2,this)">Boton 2</div>
</div>
<div id="btnform">
<label>URL</label>
<input id="burl" placeholder="http://ejemplo.com/accion">
<div class="row2">
<div>
<label>Metodo</label>
<select id="bmethod" onchange="togglePayload()">
<option value="0">GET</option>
<option value="1">POST</option>
</select>
</div>
<div>
<label>Timeout (seg)</label>
<input id="btimeout" type="number" min="1" max="30" value="5" placeholder="5">
</div>
</div>
<div id="payload-row" class="payload-row">
<label>Payload (JSON)</label>
<input id="bpayload" placeholder='{"key":"value"}'>
</div>
<button class="btn btn-save" onclick="saveBtn()">Guardar Boton</button>
<button class="btn btn-test" onclick="testBtn()">Probar Boton</button>
<div id="bmsg" class="msg"></div>
</div>
</div>

<div class="card">
<h2>Administracion</h2>
<label>Usuario</label>
<input id="auser" placeholder="admin">
<label>Nueva contraseña</label>
<div class="pass-wrap">
<input id="apass" type="password" placeholder="Nueva contraseña">
<button class="pass-toggle" type="button" onclick="togVis('apass')">👁</button>
</div>
<label>Confirmar contraseña</label>
<div class="pass-wrap">
<input id="apass2" type="password" placeholder="Repetir contraseña">
<button class="pass-toggle" type="button" onclick="togVis('apass2')">👁</button>
</div>
<button class="btn btn-admin" onclick="saveAdmin()">Cambiar credenciales</button>
<div id="amsg" class="msg"></div>
</div>
</div>

<script>
var authHdr='';
var curBtn=1;

function doLogin(){
  var u=document.getElementById('luser').value;
  var p=document.getElementById('lpass').value;
  if(!u||!p){showMsg('lmsg',false,'Introduce usuario y contraseña');return;}
  authHdr='Basic '+btoa(u+':'+p);
  fetch('/api/verify',{headers:{'Authorization':authHdr}})
  .then(function(r){
    if(r.ok){
      document.getElementById('login').classList.add('hidden');
      document.getElementById('app').classList.remove('hidden');
      document.getElementById('auser').value=u;
      loadBtn(1);
    } else {
      authHdr='';
      showMsg('lmsg',false,'Usuario o contraseña incorrectos');
    }
  }).catch(function(){showMsg('lmsg',false,'Error de conexion')});
}

function authFetch(url,opts){
  opts=opts||{};
  opts.headers=opts.headers||{};
  opts.headers['Authorization']=authHdr;
  return fetch(url,opts);
}

function switchTab(n,el){
  curBtn=n;
  document.querySelectorAll('.tab').forEach(function(t){t.classList.remove('active')});
  el.classList.add('active');
  loadBtn(n);
}
function togVis(id){
  var p=document.getElementById(id);
  p.type=p.type==='password'?'text':'password';
}
function togglePayload(){
  var r=document.getElementById('payload-row');
  r.classList.toggle('show',document.getElementById('bmethod').value==='1');
}
function showMsg(id,ok,txt){
  var m=document.getElementById(id);
  m.className='msg '+(ok?'ok':'err');m.textContent=txt;
  setTimeout(function(){m.className='msg';},4000);
}
function scanWifi(){
  var l=document.getElementById('wlist');
  l.innerHTML='<div style="text-align:center;padding:10px;color:#aaa">Buscando...</div>';
  authFetch('/api/scan').then(function(r){return r.json()}).then(function(data){
    if(!data.length){l.innerHTML='<div style="text-align:center;padding:10px;color:#aaa">No se encontraron redes</div>';return;}
    l.innerHTML='';
    data.forEach(function(ap){
      var d=document.createElement('div');d.className='wifi-item';
      var sig=ap.rssi>-50?'▂▄▆█':ap.rssi>-70?'▂▄▆_':'▂▄__';
      var lock=ap.auth>0?'🔒':'';
      d.innerHTML='<span class="name">'+ap.ssid+'</span><span class="rssi">'+sig+' '+ap.rssi+'dBm</span><span class="lock">'+lock+'</span>';
      d.onclick=function(){
        document.querySelectorAll('.wifi-item').forEach(function(i){i.classList.remove('sel')});
        d.classList.add('sel');
        document.getElementById('ssid').value=ap.ssid;
        document.getElementById('pass').focus();
      };
      l.appendChild(d);
    });
  }).catch(function(){l.innerHTML='';showMsg('wmsg',false,'Error al buscar');});
}
function saveWifi(){
  var s=document.getElementById('ssid').value,p=document.getElementById('pass').value;
  if(!s){showMsg('wmsg',false,'Introduce un SSID');return;}
  authFetch('/api/wifi',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify({ssid:s,pass:p})
  }).then(function(r){return r.json()}).then(function(d){
    showMsg('wmsg',d.ok,'ok' in d && d.ok?'Guardado. Reiniciando...':'Error');
    if(d.ok)setTimeout(function(){location.reload()},3000);
  }).catch(function(){showMsg('wmsg',false,'Error de conexion')});
}
function getBtnData(){
  return {
    id:curBtn,
    url:document.getElementById('burl').value,
    method:parseInt(document.getElementById('bmethod').value),
    payload:document.getElementById('bpayload').value||'',
    timeout:parseInt(document.getElementById('btimeout').value||'5')*1000
  };
}
function saveBtn(){
  var d=getBtnData();
  if(!d.url){showMsg('bmsg',false,'Introduce una URL');return;}
  authFetch('/api/btn',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify(d)
  }).then(function(r){return r.json()}).then(function(d){
    showMsg('bmsg',d.ok,'ok' in d && d.ok?'Boton '+curBtn+' guardado':'Error');
  }).catch(function(){showMsg('bmsg',false,'Error de conexion')});
}
function testBtn(){
  var d=getBtnData();
  if(!d.url){showMsg('bmsg',false,'Introduce una URL');return;}
  showMsg('bmsg',true,'Probando...');
  authFetch('/api/test',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify(d)
  }).then(function(r){return r.json()}).then(function(d){
    if(d.status<0){
      showMsg('bmsg',false,'Error de conexion (status: '+d.status+')');
    } else if(d.ok){
      showMsg('bmsg',true,'OK - Status: '+d.status);
    } else {
      showMsg('bmsg',false,'Error - Status: '+d.status);
    }
  }).catch(function(){showMsg('bmsg',false,'Error de conexion')});
}
function loadBtn(n){
  authFetch('/api/btn?id='+n).then(function(r){return r.json()}).then(function(d){
    document.getElementById('burl').value=d.url||'';
    document.getElementById('bmethod').value=d.method||0;
    document.getElementById('bpayload').value=d.payload||'';
    var t=d.timeout||5000;
    document.getElementById('btimeout').value=Math.round(t/1000);
    togglePayload();
  }).catch(function(){
    document.getElementById('burl').value='';
    document.getElementById('bmethod').value=0;
    document.getElementById('bpayload').value='';
    document.getElementById('btimeout').value=5;
    togglePayload();
  });
}
function saveAdmin(){
  var u=document.getElementById('auser').value;
  var p=document.getElementById('apass').value;
  var p2=document.getElementById('apass2').value;
  if(!u){showMsg('amsg',false,'Introduce un usuario');return;}
  if(!p){showMsg('amsg',false,'Introduce una contraseña');return;}
  if(p!==p2){showMsg('amsg',false,'Las contraseñas no coinciden');return;}
  if(p.length<4){showMsg('amsg',false,'Minimo 4 caracteres');return;}
  authFetch('/api/admin',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify({user:u,pass:p})
  }).then(function(r){return r.json()}).then(function(d){
    if(d.ok){
      authHdr='Basic '+btoa(u+':'+p);
      showMsg('amsg',true,'Credenciales actualizadas');
      document.getElementById('apass').value='';
      document.getElementById('apass2').value='';
    } else {
      showMsg('amsg',false,'Error al guardar');
    }
  }).catch(function(){showMsg('amsg',false,'Error de conexion')});
}

document.getElementById('lpass').addEventListener('keyup',function(e){
  if(e.key==='Enter')doLogin();
});
</script>
</body></html>
)rawliteral";
