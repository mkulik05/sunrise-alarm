
jsIncludings = '<script src="index.js"></script>'
cssIncludings = '<link rel="stylesheet" href="index.css">'

mainHTML = "web/index.html"
css = "web/index.css"
js = "web/index.js"

mainFILE = open(mainHTML)
content = mainFILE.read().replace("  ", " ")

jsCode = open(js).read()
content = content.replace(jsIncludings, "<script>{}</script>".format(jsCode))

cssStylesheet = open(css).read()
content = content.replace(cssIncludings, "<style>{}</style>".format(cssStylesheet.replace("  ", " ")))

content = content.replace("\n", "\\n")

res = open("main.cpp")
script = res.read()
res.close()
res = open("main.cpp", 'w')
script = script.replace(script[(script.index("// START")+8):(script.index("// END"))], '\nString htmlData = "{}";\n'.format(content.replace('"', "'")), 1)
res.write(script)

res.close()