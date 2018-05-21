function writeCSS(strPath)
{
	if (navigator.appName == "Microsoft Internet Explorer") {
		var sVer = navigator.appVersion;
		sVer = sVer.substring(0, sVer.indexOf("."));
		if (sVer >= 4) {
			document.writeln('<SCRIPT FOR=reftip EVENT=onclick>window.event.cancelBubble = true;</SCRIPT>');
			strPath += '4.css';
		}
		else
			strPath += '3.css';
	}
	else
		strPath += "n.css";

	document.writeln('<LINK REL="stylesheet" HREF="' + strPath + '">');
}

function bodyOnLoad()
{
	// Return if the browser isn't IE4 or later.
	if (navigator.appName != "Microsoft Internet Explorer")
		return false;
	var sVersion = navigator.appVersion;
	sVersion = sVersion.substring(0, sVersion.indexOf(".", 0));
	if (sVersion < 4)
		return false;

	// Find the syntax block, if any.
	var syn = findSyn();
	if (!syn)
		return;

	// Get the syntax HTML.
	var strSyn = syn.outerHTML;
	var ichStart = strSyn.indexOf('>', 0) + 1;

	// Find the parameter list, if any.
	var dl = findDL();
	if (dl) {
		var terms = dl.children.tags("DT");
		var iTerm;
		for (iTerm = 0; iTerm < terms.length; iTerm++) {
			var words = terms[iTerm].innerText.replace(/\[.+\]/g, " ").replace(/,/g, " ").split(" ");
			var iWord;
			for (iWord = 0; iWord < words.length; iWord++) {
				var word = words[iWord];
				if (word.length > 0) {
					var ichMatch = findTerm(strSyn, ichStart, word);
					while (ichMatch > 0) {
						var strTag = '<A HREF="" ONCLICK="showTip(this)" CLASS="synParam">' + word + '</A>';

						strSyn =
							strSyn.slice(0, ichMatch) +
							strTag +
							strSyn.slice(ichMatch + word.length);

						ichMatch = findTerm(strSyn, ichMatch + strTag.length, word);
					}
				}
			}
		}
	}

	if (findReturn()) {
		var txtLine, aLines, iLine, iWord, strTok;
		txtLine = syn.innerText;
		aLines = txtLine.split("\n");
		for (iLine = 0; iLine < aLines.length; iLine++) {
			var aWords = aLines[iLine].split(" ");
			if (aWords.length > 0) {
				for (iWord = 0; iWord < aWords.length; iWord++)
					if (aWords[iWord].length > 0)
						break;
				if (iWord < aWords.length) {
					if (isalnum(aWords[iWord].charAt(0))) {
						strTok = aWords[iWord];
						break;
					}
				}
			}
		}
		if (strTok) {
			var ichMatch = strSyn.indexOf(strTok, ichStart);
			if (ichMatch > 0) {
				var strTag = '<A HREF="" ONCLICK="showTip(this)" CLASS="synRetVal">' + strTok + '</A>';
				strSyn =
					strSyn.slice(0, ichMatch) +
					strTag +
					strSyn.slice(ichMatch + strTok.length);
			}
		}
	}

	// Replace the syntax block with our modified version.
	syn.outerHTML = strSyn;

	// Insert the reftip element.
	document.body.insertAdjacentHTML(
		'BeforeEnd',
		'<DIV ID=reftip CLASS=reftip STYLE="position:absolute;visibility:hidden;overflow:visible;"></DIV>'
		);
}

function isBold(str, index)
{
	var iStart = str.lastIndexOf("<B>", index);
	if (iStart >= 0) {
		var iEnd = str.lastIndexOf("</B>", index);
		if (iEnd < iStart)
			return true;
	}
	return false;
}

function isItalic(str, index)
{
	var iStart = str.lastIndexOf("<I>", index);
	if (iStart >= 0) {
		var iEnd = str.lastIndexOf("</I>", index);
		if (iEnd < iStart)
			return true;
	}
	return false;
}

function findTerm(strSyn, ichPos, strTerm)
{
	var ichMatch = strSyn.indexOf(strTerm, ichPos);
	while (ichMatch >= 0) {
		if ((ichMatch == 0 || !isalnum(strSyn.charAt(ichMatch - 1))) && 
				!isalnum(strSyn.charAt(ichMatch + strTerm.length))) {
			var ichComment = strSyn.indexOf("/*", ichPos);
			while (ichComment >= 0) {
				if (ichComment > ichMatch) { 
					ichComment = -1;
					break; 
				}
				var ichEnd = strSyn.indexOf("*/", ichComment);
				if (ichEnd < 0 || ichEnd > ichMatch)
					break;
				ichComment = strSyn.indexOf("/*", ichEnd);
			}
			if (ichComment < 0) {
				ichComment = strSyn.indexOf("//", ichPos);
				while (ichComment >= 0) {
					if (ichComment > ichMatch) {
						ichComment = -1;
						break; 
					}
					var ichEnd = strSyn.indexOf("\n", ichComment);
					if (ichEnd < 0 || ichEnd > ichMatch)
						break;
					ichComment = strSyn.indexOf("//", ichEnd);
				}
			}
			if (ichComment < 0)
				break;
		}
		ichMatch = strSyn.indexOf(strTerm, ichMatch + strTerm.length);
	}
	return ichMatch;
}

function findSyn()
{
	var col = document.body.children.tags("PRE");
	if (!col.length)
		return null;

	var syn = col[0];
	if (syn.className && syn.className == "syntax")
		return syn;

	var rmh = document.body.children.tags("H4");
	var i;
	for (i = 0; i < rmh.length; i++) {
		if (rmh[i].innerText.indexOf("QuickInfo", 0) >= 0)
			return syn;
	}

	return null;	
}

function findDL()
{
	var col = document.body.children.tags("DL");
	return (col.length > 0) ? col[0] : null;
}

function findReturn()
{
	var col = document.body.children.tags("H4");
	var i = 0;
	for (i = 0; i < col.length; i++) {
		if (col[i].innerText.substring(0, 6) == "Return")
			return col[i];
	}

	return null;
}

function isalnum(ch)
{
	return ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch == '_'));
}

function showTip(link)
{
	if (window.event) {
		window.event.returnValue = false;
		window.event.cancelBubble = true;
	}

	var tip = document.all.reftip;
	if (!tip || !link)
		return;

	if (window.linkElement)
		hideTip();

	window.linkElement = link;

	// Hide the tip if necessary and initialize its size.
	tip.style.visibility = "hidden";
	tip.style.pixelWidth = 260;
	tip.style.pixelHeight = 24;

	// Find the link target and initialize the tip's contents.
	var sTip;
	if (link.className == "synRetVal") {
		var elemHead = null;
		var iFirst = 0;
		var iLast = 0;

		var iElem;
		for (iElem = 0; iElem < document.body.children.length; iElem++) {
			var head = document.body.children[iElem];
			if (head.tagName == "H4" && head.innerText.substring(0,6) == "Return") {
				elemHead = head;
				iFirst = iElem + 1;
				for (iLast = iFirst; document.body.children[iLast + 1] != tip; iLast++)
					if (document.body.children[iLast + 1].tagName == "H4")
						break;
				break;
			}
		}

		if (!elemHead)
			return;

		window.linkTarget = elemHead;

		sTip = '<A HREF="javascript:jumpParam()"><B>';
		sTip += elemHead.innerHTML + "</B></A>";
		for (iElem = iFirst; iElem <= iLast; iElem++)
			sTip += document.body.children[iElem].outerHTML;
	}
	else {
		var elemDT = null;
		var elemDD = null;

		var iElem;
		for (iElem = 0; iElem < document.body.children.length; iElem++) {
			var dl = document.body.children[iElem];
			if (dl.tagName == "DL") {
				for (iElem = 0; iElem + 1 < dl.children.length; iElem++) {
					var dt = dl.children[iElem];
					if (dt.tagName == "DT") {
						if (findTerm(dt.innerText, 0, link.innerText) >= 0) {
							elemDT = dt;
							elemDD = dl.children[iElem + 1];
							break;
						}
					}
				}
				break;
			}
		}
		
		if (!elemDT)
			return;

		window.linkTarget = elemDT;

		sTip = '<DL><DT><A HREF="javascript:jumpParam()">';
		sTip += elemDT.innerHTML + "</A></DT>";
		sTip += elemDD.outerHTML + "</DL>";
	}
	tip.innerHTML = sTip;

	// Position the tip after it's updated.
	window.setTimeout("moveTip()", 0);
}

function jumpParam()
{
	hideTip();

	window.linkTarget.scrollIntoView();
	document.body.scrollLeft = 0;

	flash(3);
}

function flash(c)
{
	window.linkTarget.style.background = (c & 1) ? "#FFFF80" : "";
	if (c)
		window.setTimeout("flash(" + (c-1) + ")", 200);
}

function moveTip()
{
	var tip = document.all.reftip;
	var link = window.linkElement;
	if (!tip || !link)
		return; //error

	var w = tip.offsetWidth;
	var h = tip.offsetHeight;

	if (w > tip.style.pixelWidth) {
		tip.style.pixelWidth = w;
		window.setTimeout("moveTip()", 0);
		return;
	}

	var maxw = document.body.clientWidth;
	var maxh = document.body.clientHeight;

	if (h > maxh) {
		if (w < maxw) {
			w = w * 3 / 2;
			tip.style.pixelWidth = (w < maxw) ? w : maxw;
			window.setTimeout("moveTip()", 0);
			return;
		}
	}

	var x,y;

	var linkLeft = link.offsetLeft - document.body.scrollLeft;
	var linkRight = linkLeft + link.offsetWidth;

	var linkTop = link.offsetTop - document.body.scrollTop;
	var linkBottom = linkTop + link.offsetHeight;

	var cxMin = link.offsetWidth - 24;
	if (cxMin < 16)
		cxMin = 16;

	if (linkLeft + cxMin + w <= maxw) {
		x = maxw - w;
		if (x > linkRight + 8)
			x = linkRight + 8;
		y = maxh - h;
		if (y > linkTop)
			y = linkTop;
	}
	else if (linkBottom + h <= maxh) {
		x = maxw - w;
		if (x < 0)
			x = 0;
		y = linkBottom;
	}
	else if (w <= linkRight - cxMin) {
		x = linkLeft - w - 8;
		if (x < 0)
			x = 0;
		y = maxh - h;
		if (y > linkTop)
			y = linkTop;
	}
	else if (h <= linkTop) {
		x = maxw - w;
		if (x < 0)
			x = 0;
		y = linkTop - h;
	}
	else if (w >= maxw) {
		x = 0;
		y = linkBottom;
	}
	else {
		w = w * 3 / 2;
		tip.style.pixelWidth = (w < maxw) ? w : maxw;
		window.setTimeout("moveTip()", 0);
		return;
	}
	
	link.style.background = "#FFFF80";
	tip.style.pixelLeft = x + document.body.scrollLeft;
	tip.style.pixelTop = y + document.body.scrollTop;
	tip.style.visibility = "visible";
}

function bodyOnClick()
{
	var tip = document.all.reftip;
	if (tip && tip.style.visibility == "visible")
		hideTip();
}

function hideTip()
{
	if (window.linkElement) {
		window.linkElement.style.background = "";
		window.linkElement = null;
	}

	var tip = document.all.reftip;
	if (tip) {
		tip.style.visibility = "hidden";
		tip.innerHTML = "";
	}
}

