var fn_hello = async (ctx, next) => {
    var name = ctx.params.name;
    const start = new Date().getTime(); // 当前时间
    ctx.response.body = `<h1>Hello, ${name}!, ${start}!</h1>
                            <p><a href="/hello/${name}">刷新</a></p>
                            <p><a href="/">首页</a></p>`;
};

module.exports = {
    'GET /hello/:name': fn_hello
};