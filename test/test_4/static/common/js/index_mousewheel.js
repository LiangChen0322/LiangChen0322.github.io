$(function() {
	/*滚动面板的高度*/
	var win_w = $(window).height();
	var main_h = $(".head").height() + $(".index_mian").height();
	var scr_h = win_w - main_h;
	var scr1_mid = $(".scroll_top .middle").innerHeight();
	var scr1_pad = (scr_h - scr1_mid)/2;
	var fir_h = $(".first_mousewheel").innerHeight();
	$(".scroll_top").css({"height":scr1_pad + scr1_mid - 1,"padding-top":scr1_pad});
	function first_scroll(){
		$(".index_scroll").show();
		var fir_mind_h = $(".index_scroll .scroll_box:first").find(".middle").height();
		var pad = (win_w - fir_mind_h)/2;
		$(".index_scroll .scroll_box:first").css({"padding-top":pad,"height":pad + fir_mind_h,"bottom":(-(pad*2 + fir_mind_h))*2});
		$(".first_mousewheel").addClass("first_hide").animate({top:-fir_h});
		$(".scroll_navs").fadeIn();
		$(".index_scroll .scroll_box:first").animate({bottom:-(pad*2 + fir_mind_h)},200)
	}
	/*点击滚动事件*/
	$(".scroll_top a").click(function(){
		first_scroll();
	})
	/***************滚轮滚动***************/
	/**第一次向下滚**/
	$('.first_mousewheel').mousewheel(function(event, delta, deltaX, deltaY) {
			var o = '';
			if (delta > 0){
				log( o );
			}else if (delta < 0){
				o = '.first_mousewheel: down ('+delta+')';
				first_scroll();
			}if( o != '' ){
				log( o );
			}
			event.preventDefault();
	});
	/**第一屏向上滚**/
	$('.index_scroll .scroll_box:first').mousewheel(function(event, delta, deltaX, deltaY) {
			var o = '';
			if (delta > 0){
				o = '.first_mousewheel: up ('+delta+')';
				var fir_mind_h = $(".index_scroll .scroll_box:first").find(".middle").height();
				var pad = (win_w - fir_mind_h)/2;
				$(".first_mousewheel").animate({top:"0"},200);
				$(".index_scroll .scroll_box:first").animate({bottom:(-(pad*2 + fir_mind_h))*2},1000);
				$(".scroll_navs").fadeOut();
				t = setTimeout("$('.index_scroll').hide()",500);
			}else if (delta < 0){
				log( o );
			}if( o != '' ){
				log( o );
			}
			event.preventDefault();
	});
	/**第二屏及其之后向下滚**/
	$('.scroll_box').live("mousewheel",function(event, delta, deltaX, deltaY) {
			var o = '';
			if (delta > 0){
				log( o );
			}else if (delta < 0){
				o = '.first_mousewheel: down ('+delta+')';
				$(this).next(".scroll_box").show();
				var mind_h = $(this).next(".scroll_box").find(".middle").height();
				var pad = (win_w - mind_h)/2;
				var s = $(this).next(".scroll_box").index();
				$(this).next(".scroll_box").css({"bottom":(-(pad*2 + mind_h))*2});
				$(this).next(".scroll_box").css({"padding-top":pad,"height":pad + mind_h}); 
				$(this).next(".scroll_box").animate({bottom:-(pad*2 + mind_h)},1000).addClass("current").siblings(".scroll_box").removeClass("current");
				$(".scroll_navs a").eq(s).addClass("current").siblings().removeClass("current");
			}if( o != '' ){
				log( o );
			}
			event.preventDefault();
	});
	/**第二屏向上滚**/
	$('.current').live("mousewheel",function(event, delta, deltaX, deltaY) {
			var o = '';
			if (delta > 0){
				o = '.first_mousewheel: up ('+delta+')';
				var mind_h = $(this).prev(".scroll_box").find(".middle").height();
				var pad = (win_w - mind_h)/2;
				var s = $(this).prev(".scroll_box").index();
				$(this).animate({bottom:(-(pad*2 + mind_h))*2,"z-index":"10"},1000);
				$(this).css("z-index","0");
				$(this).prev(".scroll_box").addClass("current").siblings(".scroll_box").removeClass("current");
				if(s<0){
					$(".scroll_navs a:first").addClass("current").siblings().removeClass("current");
				}else{
					$(".scroll_navs a").eq(s).addClass("current").siblings().removeClass("current");
				}
			}else if (delta < 0){
				log( o );
			}if( o != '' ){
				log( o );
			}
			event.preventDefault();
	});
	function log(msg) {
		$('.mousewheel_box').scrollTop(999999);
	};
});