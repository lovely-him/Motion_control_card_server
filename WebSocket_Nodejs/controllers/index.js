var fn_index = async (ctx, next) => {
    ctx.response.body = `<h1>运动控制卡 - 云平台</h1>
        <form action="/signin" method="post">
            <p>账号: <input name="name" value="him"></p>
            <p>密码: <input name="password" type="password"></p>
            <p><input type="submit" value="登陆"></p>
        </form>
        
        <p><a href="/hello/him">测试</a></p>`;
};

var fn_signin = async (ctx, next) => {
    var
        name = ctx.request.body.name || '',
        password = ctx.request.body.password || '';
    console.log(`signin with name: ${name}, password: ${password}`);
    // if (name === 'him' && password === '123') {
    if (password === '123') {
        ctx.response.body = `<h1>运动控制卡 - 云平台</h1>
            <h1>登陆成功, 欢迎 ${name} !</h1>
            <form action="/signin/him" method="post">
                <p><input type="submit" value="开始通讯"></p>
            </form>`;
    } else {
        ctx.response.body = `<h1>登陆失败!</h1>
        <p><a href="/">返回</a></p>`;
    }
};

module.exports = {
    'GET /': fn_index,
    'POST /signin': fn_signin
};