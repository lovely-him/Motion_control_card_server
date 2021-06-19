
var rx_data = 0;

// 搭建websocket服务器
const ws = require("nodejs-websocket");
var _server = ws.createServer(conn => {
	// 接收客户端返回的数据
	conn.on("text", function(str) {
        rx_data = str;
		console.log(str, "接收客户端传过来的值");
	});

    //客户端关闭连接
	conn.on("close", function() {
		
	});

	conn.on("error", function(err) {
		//error
		console.log(err, "连接报错");
	});
});
// 定义端口为2002【端口自己随意定义】
const port = 8880;
_server.listen(port, function() {
	
	console.log("连接成功 8880")
	console.log('listening on websocketServer');
})

var fn_him = async (ctx, next) => {
    ctx.response.body = `<h1>运动控制卡 - 云平台</h1>
            <h1>${rx_data}</h1>
            <form action="/signin/him" method="post">
                <p><input type="submit" value="刷新"></p>
            </form>
            <p><a href="/">返回</a></p>`;
};

module.exports = {
    'POST /signin/him': fn_him
};