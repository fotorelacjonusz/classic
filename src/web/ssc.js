// ==UserScript==
// @match *://www.skyscrapercity.com/*
// @run-at document-end
// ==/UserScript==

(function(){
	var replyDialog

	console.log("Connecting to Fotorelacjonusz…")
	initWebChannel()

	/**
	 * Waits till QWebChannel constructor is available (script load order is
	 * quite random), and then creates a new instance, passing channelReady
	 * function as a callback.
	 */
	function initWebChannel() {
		if (typeof(QWebChannel) === "function") {
			new QWebChannel(qt.webChannelTransport, channelReady)
		} else {
			setTimeout(initWebChannel, 50)
		}
	}

	function channelReady(channel) {
		console.log("Established connection to Fotorelacjonusz.")

		replyDialog = channel.objects.replyDialog
		replyDialog.forumPageLoaded(location.href)
		detectPageType()
	}

	////////////////

	/**
	 * Detects page type by its location, and calls appropriate handler
	 * with no arguments.
	 */
	function detectPageType() {
		switch(window.location.pathname) {
			case "/newreply.php": itsANewReplyPage() ; break
			case "/showthread.php": itsAShowThreadPage() ; break
		}
	}

	/**
	 * This is a handler for newreply.php page.  Being on this page means one of
	 * two things: that new post can be submitted or that submission of previous
	 * post has failed (typically due to throttling).
	 *
	 * This handler tells these two apart, and notifies replyDialog accordingly.
	 */
	function itsANewReplyPage() {
		if (isThrottled()) {
			replyDialog.forumReplySubmissionFailed()
		} else {
			scrollToForm()
			waitForReplyBody(() => obtainPostBody((body) => postReply(body)))
		}
	}

	function itsAShowThreadPage() {
		let replyBtnImgQuery = "html > body > center > div > div.page " +
			"> div > table > tbody > tr > td > a > img[alt=Reply]"
		let replyBtnImg = document.querySelector(replyBtnImgQuery)
		let replyUrl = replyBtnImg.parentElement.href

		replyDialog.forumThreadVisited(replyUrl)
	}

	/**
	 * On newreply.php page, detect whether previous submission was throttled.
	 * Return true or false.
	 */
	function isThrottled() {
		let errorTitle = "The following errors occurred with your submission"

		for (let candidate of document.querySelectorAll(".tcat")) {
			if (candidate.innerText.includes(errorTitle)) {
				return true
			}
		}

		return false
	}

	function scrollToForm() {
		let form = document.querySelector("form[name=vbform] .smallfont a")
		form.scrollIntoView()
	}

	function waitForReplyBody(callback) {
		replyDialog.isNextPostAvailable((bool) => {
			if (bool) {
				callback()
			} else {
				setTimeout(() => { waitForReplyBody(callback) }, 100)
			}
		})
	}

	function obtainPostBody(callback) {
		replyDialog.obtainNextPost(callback)
	}

	function postReply(postBody) {
		let msgArea = document.querySelector("textarea[name=message]")

		msgArea.value = postBody
		msgArea.closest("form").submit()
	}
})()
