<!doctype html>
<html>

<head>
<script type="text/javascript" src="demo/example_anim.js"></script>
<script type="text/javascript">
var delay_scale = 0.7
var timer = null

var animate = function(img, timeline, element)
{
	var i = 0

	var run_time = 0
	for (var j = 0; j < timeline.length - 1; ++j)
		run_time += timeline[j].delay

	var f = function()
	{
		var frame = i++ % timeline.length
		var delay = timeline[frame].delay * delay_scale
		var blits = timeline[frame].blit

		var ctx = element.getContext('2d')

		for (j = 0; j < blits.length; ++j)
		{
			var blit = blits[j]
			var sx = blit[0]
			var sy = blit[1]
			var w = blit[2]
			var h = blit[3]
			var dx = blit[4]
			var dy = blit[5]
			ctx.drawImage(img, sx, sy, w, h, dx, dy, w, h)
		}

		timer = window.setTimeout(f, delay)
	}

	if (timer) window.clearTimeout(timer)
	f()
}

var animate_fallback = function(img, timeline, element)
{
	var i = 0

	var run_time = 0
	for (var j = 0; j < timeline.length - 1; ++j)
		run_time += timeline[j].delay

	var f = function()
	{
		if (i % timeline.length == 0)
		{
			while (element.hasChildNodes())
				element.removeChild(element.lastChild)
		}

		var frame = i++ % timeline.length
		var delay = timeline[frame].delay * delay_scale
		var blits = timeline[frame].blit

		for (j = 0; j < blits.length; ++j)
		{
			var blit = blits[j]
			var sx = blit[0]
			var sy = blit[1]
			var w = blit[2]
			var h = blit[3]
			var dx = blit[4]
			var dy = blit[5]

			var d = document.createElement('div')
			d.style.position = 'absolute'
			d.style.left = dx + "px"
			d.style.top = dy + "px"
			d.style.width = w + "px"
			d.style.height = h + "px"
			d.style.backgroundImage = "url('" + img.src + "')"
			d.style.backgroundPosition = "-" + sx + "px -" + sy + "px"

			element.appendChild(d)
		}

		timer = window.setTimeout(f, delay)
	}

	if (timer) window.clearTimeout(timer)
	f()
}

function set_animation(img_url, timeline, canvas_id, fallback_id)
{
	var img = new Image()
	img.onload = function()
	{
		var canvas = document.getElementById(canvas_id)
		if (canvas && canvas.getContext)
			animate(img, timeline, canvas)
		else
			animate_fallback(img, timeline, document.getElementById(fallback_id))
	}
	img.src = img_url
}
</script>
</head>

<body>

<div><canvas id="anim_target" class="anim_target" width="800" height="450">
<div id="anim_fallback" class="anim_target" style="width: 800px; height: 450px; position: relative;"></div>
<p></canvas></div>

<script>
set_animation("demo/example_packed.png", example_timeline, 'anim_target', 'anim_fallback');
</script>

##Linux平台下的A股神器

###依赖列表
使用C++(11)开发,第三方库如下:

1. Boost
2. libcurl
3. libev
4. ncurses(w)

###编译
`build/bin/`下有编译好的程序,手动编译如下:

1. `mkdir build`
2. `cd build`
3. `cmake ../`
4. `make`

###使用指南
**数据位置**

    build/bin/data/code

自选股/大盘所有股票存放(可在配置文件中更改)

**命令大全**

    -h [ --help ]         显示帮助信息
    -a [ --code ] arg     查看个股信息，代码用','分开
    -c [ --config ] arg   设置配置XML配置文件(示例:build/bin/ccstock.xml)
    -d [ --data ] arg     显示板块行情(板块名称sh/sz/cyb/zxb/all)
    -m [ --market ]       显示大盘行情(沪市/深市/中小板/创业板)
    -t [ --top ] arg      筛选前几个信息(--top 10, 根据排序规则)
    -o [ --order ] arg    排序规则如下：
                          xj    现价(高到低,下同)
                          zdf   涨跌幅
                          cjl   成交量
                          cje   成交额
                          xji   现价(低到高,下同)
                          zdfi   涨跌幅
                          cjli   成交量
                          cjei   成交额
    --add-option arg      追加自选股,股票代码用','分开
    --del-option arg      删除自选股,股票代码用','分开
    -O [ --show-option ]  查看自选股

**Demo**

查看创业板(cyb)现价前20的股票

    ./ccstock -c ccstock.xml -d cyb -t 20 -o xj

###版本更新

####2015-07-13. Verion 0.2
1. 增加通过简拼查询功能
2. 加入ncurses实时刷新 
3. 完善错误处理

####2015-07-01. Verion 0.1
1. 实现基本行情查看

</body>
