const char index_html[] = R"rawliteral(
<!DOCTYPE html><html><head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
<title>SmartButton</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,"Helvetica Neue",Arial,sans-serif;background:#212529;color:#dee2e6;min-height:100vh;font-size:1rem;line-height:1.5}
.header{background:#2b3035;padding:14px 16px;text-align:center;border-bottom:1px solid #495057;position:sticky;top:0;z-index:50}
.header h1{font-size:1.25rem;color:#fff;margin:0;font-weight:600;letter-spacing:.5px}
.nav{position:fixed;bottom:0;left:0;right:0;background:#2b3035;border-top:1px solid #495057;display:flex;z-index:50;padding-bottom:env(safe-area-inset-bottom)}
.nav-item{flex:1;text-align:center;padding:10px 0 8px;cursor:pointer;color:#6c757d;font-size:.72em;transition:all .2s}
.nav-item .ico{font-size:1.4em;display:block;margin-bottom:3px}
.nav-item.act{color:#0d6efd;background:rgba(13,110,253,.08)}
.nav-item:active{background:rgba(13,110,253,.15)}
.sec{display:none;padding:16px;padding-bottom:90px;max-width:540px;margin:0 auto}
.sec.act{display:block}
.card{background:#2b3035;border:1px solid #495057;border-radius:.5rem;padding:20px;margin-bottom:16px}
.card h2{font-size:.95rem;font-weight:600;margin-bottom:14px;color:#fff;border-bottom:1px solid #495057;padding-bottom:10px}
label{display:block;font-size:.875rem;color:#adb5bd;margin-bottom:6px;font-weight:500}
input,select{width:100%;padding:10px 14px;border:1px solid #495057;border-radius:.375rem;background:#343a40;color:#dee2e6;font-size:.9rem;margin-bottom:12px;-webkit-appearance:none;transition:border-color .15s,box-shadow .15s}
input:focus,select:focus{outline:none;border-color:#86b7fe;box-shadow:0 0 0 .2rem rgba(13,110,253,.25)}
select{cursor:pointer;background-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 16 16'%3E%3Cpath fill='none' stroke='%23adb5bd' stroke-linecap='round' stroke-linejoin='round' stroke-width='2' d='m2 5 6 6 6-6'/%3E%3C/svg%3E");background-repeat:no-repeat;background-position:right 12px center;background-size:12px;padding-right:36px}
.pw{position:relative}
.pw input{padding-right:44px}
.pw-tog{position:absolute;right:10px;top:7px;background:none;border:none;color:#6c757d;font-size:1.1em;cursor:pointer;padding:4px;width:auto;transition:color .2s}
.pw-tog:hover{color:#adb5bd}
.wifi-list{max-height:220px;overflow-y:auto;margin-bottom:12px;border-radius:.375rem}
.wifi-item{display:flex;justify-content:space-between;align-items:center;padding:10px 14px;cursor:pointer;margin-bottom:2px;background:#343a40;border:1px solid transparent;border-radius:.375rem;transition:all .15s}
.wifi-item:hover{background:#3d4349;border-color:#495057}
.wifi-item.sel{background:rgba(13,110,253,.15);border-color:#0d6efd}
.wifi-item .name{font-size:.9em;flex:1;overflow:hidden;text-overflow:ellipsis;white-space:nowrap;color:#dee2e6}
.wifi-item .rssi{font-size:.75em;color:#6c757d;margin-left:8px;white-space:nowrap}
.wifi-item .lock{font-size:.75em;margin-left:4px}
button,.btn{width:100%;padding:10px 16px;border:none;border-radius:.375rem;font-size:.9rem;font-weight:600;cursor:pointer;color:#fff;margin-top:6px;transition:all .15s;letter-spacing:.3px}
button:active,.btn:active{transform:scale(.98)}
.btn-p{background:#343a40;color:#0d6efd;border:1px solid #495057}
.btn-p:hover{background:#3d4349}
.btn-ok{background:#0d6efd;color:#fff}
.btn-ok:active{background:#0b5ed7}
.btn-test{background:transparent;color:#ffc107;border:1px solid #ffc107;margin-top:8px}
.btn-test:active{background:rgba(255,193,7,.1)}
.btn-danger{background:#dc3545;color:#fff}
.btn-danger:active{background:#bb2d3b}
.msg{text-align:center;padding:10px 14px;border-radius:.375rem;margin-top:10px;font-size:.85rem;display:none;font-weight:500}
.msg.ok{display:block;background:rgba(25,135,84,.15);color:#75b798;border:1px solid rgba(25,135,84,.3)}
.msg.err{display:block;background:rgba(220,53,69,.15);color:#ea868f;border:1px solid rgba(220,53,69,.3)}
.pl-row{display:none}
.pl-row.show{display:block}
.tabs{display:flex;gap:6px;margin-bottom:14px}
.tab{flex:1;padding:9px;text-align:center;border-radius:.375rem;background:#343a40;color:#6c757d;cursor:pointer;font-size:.85em;border:1px solid #495057;font-weight:500;transition:all .15s}
.tab.act{background:rgba(13,110,253,.15);color:#6ea8fe;border-color:#0d6efd}
.r2{display:flex;gap:10px}
.r2>div{flex:1}
.hid{display:none}
.login-box{max-width:360px;margin:80px auto 0;padding:20px}
.login-box h1{margin-bottom:28px;text-align:center;color:#fff;font-size:1.5rem;font-weight:700}
input[type="file"]{padding:8px;cursor:pointer;font-size:.85rem}
.modal-bg{display:none;position:fixed;top:0;left:0;width:100%;height:100%;background:rgba(0,0,0,.6);z-index:100;align-items:center;justify-content:center;backdrop-filter:blur(4px)}
.modal-bg.show{display:flex}
.modal{background:#2b3035;border:1px solid #495057;border-radius:.5rem;padding:24px;max-width:400px;width:90%;max-height:80vh;overflow-y:auto}
.modal h2{font-size:1rem;color:#fff;margin-bottom:16px;text-align:center;font-weight:600}
.irow{display:flex;justify-content:space-between;padding:10px 0;border-bottom:1px solid #495057;font-size:.875rem}
.irow .il{color:#6c757d}
.irow .iv{color:#dee2e6;font-family:"SFMono-Regular",Consolas,"Liberation Mono",Menlo,monospace;font-size:.85em}
.chk-row{display:flex;align-items:center;margin-top:6px;margin-bottom:14px;gap:10px}
.chk-row input[type="checkbox"]{width:20px;height:20px;margin:0;cursor:pointer;accent-color:#0d6efd;flex-shrink:0;-webkit-appearance:auto;appearance:auto}
.chk-row label{display:inline;margin:0;cursor:pointer;color:#adb5bd;font-size:.85em;line-height:1.4}
</style>
</head><body>

<div id="login" class="login-box">
<h1>🔐 SmartButton</h1>
<div class="card">
<label>Usuario</label>
<input id="luser" placeholder="admin" value="admin">
<label>Contraseña</label>
<div class="pw">
<input id="lpass" type="password" placeholder="Contraseña">
<button class="pw-tog" type="button" onclick="togVis('lpass')">🗝️</button>
</div>
<button class="btn btn-ok" onclick="doLogin()">Entrar</button>
<div id="lmsg" class="msg"></div>
</div>
</div>

<div id="app" class="hid">
<div class="header"><h1>⚡ SmartButton</h1></div>

<!-- WiFi -->
<div id="s-wifi" class="sec act">
<div class="card">
<h2>📶 WiFi a conectarse</h2>
<button class="btn btn-p" onclick="scanWifi()">Buscar redes</button>
<div id="wlist" class="wifi-list"></div>
<label>SSID de tu Red</label>
<input id="ssid" placeholder="Nombre de la red">
<label>Contraseña</label>
<div class="pw">
<input id="wpass" type="password" placeholder="Contraseña WiFi">
<button class="pw-tog" type="button" onclick="togVis('wpass')">🗝️</button>
</div>
<button class="btn btn-ok" onclick="saveWifi()">Guardar WiFi</button>
<div id="wmsg" class="msg"></div>
</div>
</div>

<!-- Botones -->
<div id="s-btn" class="sec">
<div class="card">
<h2>🔘 Botones</h2>
<div class="tabs">
<div class="tab act" onclick="swTab(1,this)">Botón 1</div>
<div class="tab" onclick="swTab(2,this)">Botón 2</div>
</div>
<label>URL</label>
<input id="burl" placeholder="http://ejemplo.com/accion">
<div class="r2">
<div><label>Método</label>
<select id="bmethod" onchange="togPL()">
<option value="0">GET</option>
<option value="1">POST</option>
</select></div>
<div><label>Timeout (seg)</label>
<input id="btimeout" type="number" min="1" max="30" value="5">
</div></div>
<label>Cooldown (seg)</label>
<input id="bcooldown" type="number" step="0.5" min="0.5" max="60" value="2">
<div id="pl-row" class="pl-row">
<label>Payload (JSON)</label>
<input id="bpayload" placeholder='{"key":"value"}'>
</div>
<button class="btn btn-ok" onclick="saveBtn()">Guardar Botón</button>
<button class="btn btn-test" onclick="testBtn()">⚡ Probar Botón</button>
<div id="bmsg" class="msg"></div>
</div>
</div>

<!-- Sistema -->
<div id="s-sys" class="sec">
<div class="card">
<h2>🌐 Estado de Red</h2>
<button class="btn btn-p" onclick="showNet()">Ver Info de Red</button>
</div>

<div class="card">
<h2>📡 Ajustes del AP Local</h2>
<label>SSID (En blanco para auto-generado)</label>
<input id="ap_ssid" placeholder="SmartButton-XXXX">
<label>Contraseña WPA2 (Min 8 chars, blanco = Abierto)</label>
<div class="pw">
<input id="ap_pass" type="password" placeholder="smartbutton">
<button class="pw-tog" type="button" onclick="togVis('ap_pass')">🗝️</button>
</div>
<div class="chk-row">
<input type="checkbox" id="pure_client">
<label for="pure_client">Modo Cliente Puro (Ocultar el AP por seguridad tras conectar exitosamente a tu red WiFi)</label>
</div>
<div class="chk-row">
<input type="checkbox" id="deep_sleep">
<label for="deep_sleep">Deep Sleep (Bajo consumo: duerme tras ejecutar acción, despierta al pulsar botón)</label>
</div>
</div>

<div class="card">
<h2>🔧 Configuración Avanzada</h2>
<div class="r2">
<div><label>Reintentos WiFi</label>
<input id="sta_retries" type="number" min="1" max="20" value="5">
</div>
<div><label>Canal AP WiFi</label>
<input id="ap_channel" type="number" min="1" max="13" value="1">
</div></div>
<div class="r2">
<div><label>Timeout wakeup (seg)</label>
<input id="wk_timeout" type="number" min="10" max="120" value="30">
</div>
<div><label>Tiempo config (seg)</label>
<input id="cfg_awake" type="number" min="30" max="600" value="180">
</div></div>
<label>Anti-rebote botones (ms)</label>
<input id="debounce" type="number" min="50" max="500" value="200">
</div>

<div class="card">
<h2>🔑 Panel y Seguridad</h2>
<label>Usuario Dashboard</label>
<input id="auser" placeholder="admin">
<label>Nueva Contraseña</label>
<div class="pw">
<input id="apass" type="password" placeholder="Nueva contraseña">
<button class="pw-tog" type="button" onclick="togVis('apass')">🗝️</button>
</div>
<label>Confirmar Contraseña</label>
<div class="pw">
<input id="apass2" type="password" placeholder="Repetir contraseña">
<button class="pw-tog" type="button" onclick="togVis('apass2')">🗝️</button>
</div>
<label>Tiempo retardo Factory Reset (seg)</label>
<input id="areset" type="number" min="3" max="60" value="8">
<button class="btn btn-danger" onclick="saveAdmin()">Guardar Ajustes y Reiniciar</button>
<div id="amsg" class="msg"></div>
</div>

<div class="card">
<h2>⚠️ Factory Reset</h2>
<p style="font-size:.85em;color:#6c757d;margin-bottom:12px">Borra toda la configuración y restaura valores de fábrica.</p>
<button class="btn btn-danger" onclick="doFactoryReset()">Factory Reset</button>
<div id="frmsg" class="msg"></div>
</div>

<div class="card">
<h2>📦 Firmware OTA</h2>
<input type="file" id="otafile" accept=".bin">
<button class="btn btn-ok" onclick="doOta()">Subir Firmware</button>
<div id="omsg" class="msg"></div>
</div>
</div>

<nav class="nav">
<div class="nav-item act" onclick="go('wifi',this)"><span class="ico">📶</span>WiFi</div>
<div class="nav-item" onclick="go('btn',this)"><span class="ico">🔘</span>Botones</div>
<div class="nav-item" onclick="go('sys',this)"><span class="ico">⚙️</span>Sistema</div>
</nav>
</div>

<div id="netmodal" class="modal-bg" onclick="if(event.target===this)closeNet()">
<div class="modal">
<h2>📡 Info de Red</h2>
<div id="netbody"><div style="text-align:center;color:#6c757d;padding:20px">Cargando...</div></div>
<button class="btn btn-p" onclick="closeNet()">Cerrar</button>
</div>
</div>

<script>
var A='',cBtn=1,curP='';
function go(s,el){
document.querySelectorAll('.sec').forEach(function(x){x.classList.remove('act')});
document.querySelectorAll('.nav-item').forEach(function(x){x.classList.remove('act')});
document.getElementById('s-'+s).classList.add('act');el.classList.add('act');
}
function doLogin(){
var u=$('luser').value,p=$('lpass').value;
if(!u||!p){msg('lmsg',0,'Introduce usuario y contraseña');return;}
A='Basic '+btoa(u+':'+p);
fetch('/api/verify',{headers:{'Authorization':A}}).then(function(r){
if(r.ok){$('login').classList.add('hid');$('app').classList.remove('hid');$('auser').value=u;curP=p;loadBtn(1);loadSys();}
else{A='';msg('lmsg',0,'Usuario o contraseña incorrectos');}
}).catch(function(){msg('lmsg',0,'Error de conexion')});
}
function af(u,o){o=o||{};o.headers=o.headers||{};o.headers['Authorization']=A;return fetch(u,o);}
function $(id){return document.getElementById(id);}
function swTab(n,el){cBtn=n;document.querySelectorAll('.tab').forEach(function(t){t.classList.remove('act')});el.classList.add('act');loadBtn(n);}
function togVis(id){var p=$(id);p.type=p.type==='password'?'text':'password';}
function togPL(){$('pl-row').classList.toggle('show',$('bmethod').value==='1');}
function msg(id,ok,t){var m=$(id);m.className='msg '+(ok?'ok':'err');m.textContent=t;setTimeout(function(){m.className='msg';},4000);}
function scanWifi(){
var l=$('wlist');l.innerHTML='<div style="text-align:center;padding:12px;color:#6c757d">Buscando...</div>';
af('/api/scan').then(function(r){return r.json()}).then(function(d){
if(!d.length){l.innerHTML='<div style="text-align:center;padding:12px;color:#6c757d">No se encontraron redes</div>';return;}
l.innerHTML='';d.forEach(function(ap){
var e=document.createElement('div');e.className='wifi-item';
var s=ap.rssi>-50?'▂▄▆█':ap.rssi>-70?'▂▄▆_':'▂▄__';
e.innerHTML='<span class="name">'+ap.ssid+'</span><span class="rssi">'+s+' '+ap.rssi+'dBm</span><span class="lock">'+(ap.auth>0?'🔒':'')+'</span>';
e.onclick=function(){document.querySelectorAll('.wifi-item').forEach(function(i){i.classList.remove('sel')});e.classList.add('sel');$('ssid').value=ap.ssid;$('wpass').focus();};
l.appendChild(e);});
}).catch(function(){l.innerHTML='';msg('wmsg',0,'Error al buscar');});
}
function saveWifi(){
var s=$('ssid').value,p=$('wpass').value;
if(!s){msg('wmsg',0,'Introduce un SSID');return;}
af('/api/wifi',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({ssid:s,pass:p})
}).then(function(r){return r.json()}).then(function(d){msg('wmsg',d.ok,d.ok?'Guardado. Reiniciando...':'Error');if(d.ok)setTimeout(function(){location.reload()},3000);
}).catch(function(){msg('wmsg',0,'Error de conexion')});
}
function gBD(){return{id:cBtn,url:$('burl').value,method:parseInt($('bmethod').value),payload:$('bpayload').value||'',timeout:parseInt($('btimeout').value||'5')*1000,cooldown:parseFloat($('bcooldown').value||'2')*1000};}
function saveBtn(){
var d=gBD();if(!d.url){msg('bmsg',0,'Introduce una URL');return;}
af('/api/btn',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(d)
}).then(function(r){return r.json()}).then(function(d){msg('bmsg',d.ok,d.ok?'Boton '+cBtn+' guardado':'Error');
}).catch(function(){msg('bmsg',0,'Error de conexion')});
}
function testBtn(){
var d=gBD();if(!d.url){msg('bmsg',0,'Introduce una URL');return;}
msg('bmsg',1,'Probando...');
af('/api/test',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(d)
}).then(function(r){return r.json()}).then(function(d){
if(d.status<0)msg('bmsg',0,'Error de conexion (status: '+d.status+')');
else if(d.ok)msg('bmsg',1,'OK - Status: '+d.status);
else msg('bmsg',0,'Error - Status: '+d.status);
}).catch(function(){msg('bmsg',0,'Error de conexion')});
}
function loadBtn(n){
af('/api/btn?id='+n).then(function(r){return r.json()}).then(function(d){
$('burl').value=d.url||'';$('bmethod').value=d.method||0;$('bpayload').value=d.payload||'';
$('btimeout').value=Math.round((d.timeout||5000)/1000);$('bcooldown').value=(d.cooldown||2000)/1000;togPL();
}).catch(function(){$('burl').value='';$('bmethod').value=0;$('bpayload').value='';$('btimeout').value=5;$('bcooldown').value=2;togPL();});
}
function loadSys(){
af('/api/admin').then(function(r){return r.json()}).then(function(d){
if(d.user)$('auser').value=d.user;
if(d.reset_time)$('areset').value=d.reset_time;
if(d.ap_ssid !== undefined)$('ap_ssid').value=d.ap_ssid;
if(d.ap_pass !== undefined)$('ap_pass').value=d.ap_pass;
if(d.pure_client !== undefined)$('pure_client').checked=d.pure_client;
if(d.deep_sleep !== undefined)$('deep_sleep').checked=d.deep_sleep;
if(d.sta_max_retries !== undefined)$('sta_retries').value=d.sta_max_retries;
if(d.ap_channel !== undefined)$('ap_channel').value=d.ap_channel;
if(d.wakeup_timeout !== undefined)$('wk_timeout').value=d.wakeup_timeout;
if(d.config_awake !== undefined)$('cfg_awake').value=d.config_awake;
if(d.debounce !== undefined)$('debounce').value=d.debounce;
}).catch(function(){});
}
function saveAdmin(){
var u=$('auser').value, p=$('apass').value, p2=$('apass2').value, r=$('areset').value;
var aps=$('ap_ssid').value, app=$('ap_pass').value, pc=$('pure_client').checked;
var ds=$('deep_sleep').checked;

if(!u){msg('amsg',0,'Introduce un usuario');return;}
if(p && p!==p2){msg('amsg',0,'Las contraseñas de admin no coinciden');return;}
if(p && p.length<4){msg('amsg',0,'Minimo 4 caracteres (admin)');return;}
if(app && app.length>0 && app.length<8){msg('amsg',0,'Clave de AP debe tener mínimo 8 caracteres');return;}

var body={user:u, reset_time:parseInt(r), ap_ssid:aps, ap_pass:app, pure_client:pc, deep_sleep:ds,
sta_max_retries:parseInt($('sta_retries').value), ap_channel:parseInt($('ap_channel').value),
wakeup_timeout:parseInt($('wk_timeout').value), config_awake:parseInt($('cfg_awake').value),
debounce:parseInt($('debounce').value)};
if(p)body.pass=p;

af('/api/admin',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(body)})
.then(function(res){return res.json()}).then(function(d){
if(d.ok){
curP=p?p:curP; A='Basic '+btoa(u+':'+curP);
msg('amsg',1,'Ajustes guardados. Reiniciando el dispositivo...');
$('apass').value='';$('apass2').value='';
setTimeout(function(){location.reload()},4000);
}else msg('amsg',0,'Error al guardar');
}).catch(function(){msg('amsg',0,'Error de conexion')});
}
function doFactoryReset(){
if(!confirm('¿Estás seguro? Se borrarán TODOS los ajustes.'))return;
msg('frmsg',1,'Reseteando...');
af('/api/factory_reset',{method:'POST'}).then(function(r){return r.json()}).then(function(d){
if(d.ok)msg('frmsg',1,'Reiniciando con valores de fábrica...');
else msg('frmsg',0,'Error');
}).catch(function(){msg('frmsg',0,'Error de conexion');});
}
function doOta(){
var f=$('otafile');if(!f.files||!f.files[0]){msg('omsg',0,'Selecciona un archivo .bin');return;}
msg('omsg',1,'Subiendo firmware... NO APAGUES');
af('/api/ota',{method:'POST',body:f.files[0]}).then(function(r){return r.json()}).then(function(d){
if(d.ok){msg('omsg',1,'Actualizado. Reiniciando...');setTimeout(function(){location.reload()},10000);}
else msg('omsg',0,'Error al actualizar');
}).catch(function(){msg('omsg',0,'Error de conexion');});
}
function showNet(){
$('netmodal').classList.add('show');
$('netbody').innerHTML='<div style="text-align:center;color:#6c757d;padding:20px">Cargando...</div>';
af('/api/netinfo').then(function(r){return r.json()}).then(function(d){
var h='',rows=[['SSID',d.ssid||'No conectado'],['RSSI',d.rssi?d.rssi+' dBm':'\u2014'],['IP',d.ip||'0.0.0.0'],['Mascara',d.mask||'\u2014'],['Gateway',d.gw||'\u2014'],['DNS',d.dns||'\u2014'],['MAC',d.mac||'\u2014'],['IP AP',d.ap_ip||'\u2014']];
rows.forEach(function(r){h+='<div class="irow"><span class="il">'+r[0]+'</span><span class="iv">'+r[1]+'</span></div>';});
$('netbody').innerHTML=h;
}).catch(function(){$('netbody').innerHTML='<div style="text-align:center;color:#ea868f;padding:20px">Error</div>';});
}
function closeNet(){$('netmodal').classList.remove('show');}
$('lpass').addEventListener('keyup',function(e){if(e.key==='Enter')doLogin();});
</script>
</body></html>
)rawliteral";
