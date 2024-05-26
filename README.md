Convert any XML/HTML to JsonML using [yxml](https://dev.yorhel.nl/yxml)

## BUILD

```sh
make html2json
```

## USAGE

```sh
cat test/basic.html | ./html2json | jq .[1].lang
"en"
# send json to a frontend (example: GTK)
curl https://news.ycombinator.com/rss | ./html2json | ./json2gtk
```

## FORMAT

<table>
<tr><td>

```html
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="utf-8" />
    <title>Basic Example</title>
    <link rel="stylesheet" />
  </head>
  <body id="home">
    <input type="text"/>
    <p>content</p>
  </body>
</html>
```

</td><td>

```jsonc
// doctype is ommited
["html",{"lang":"en"},[
    ["head", {}, [
        ["meta", {"charset": "utf-8"} ],
        ["title", {}, ["Basic Example"] ],
        ["link", {"rel": "stylesheet"} ]
    ]],
    ["body", {"id": "home"}, [
        ["input", {"type": "text"}],
        ["p", {}, ["content"]]
    ]]
]]
```

</td></tr>
</table>

# HTML5 support (WIP)

yxml was added XHTML and HTML5 using:
- [x] migrate `yxml_ret_t` to bitfield enum so multiple state can be returned (example : parsing `>` in `<p hidden>` will return `ATTREND|ELEMSTART`)
- [x] accept lowercase `<!doctype `
- [x] read `<script>`, `<style>` content as raw data until matching closing tag id found 
- [ ] accept unquoted attribute value `<form method=GET>`
- [ ] accept value-less attribute `<p hidden id=p>`
- [ ] handle [void elements](https://developer.mozilla.org/en-US/docs/Glossary/Void_element) as self-closed (`<img>` will internaly generate `<img></img>`), so alwo ignore end-tag of void elements (ex: `</img>`)