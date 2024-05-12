Convert any XML/HTML to JsonML

## BUILD

```sh
make html2json
```

## USAGE

```sh
cat test/basic.html | ./html2json | jq .[1].lang
"en"
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

# LIMITATIONS

parsing is done by [yxml](https://dev.yorhel.nl/yxml) with the following changes for HTML support:
- migrate `yxml_ret_t` to bitfield enum so multiple state can be returned (example : parsing `>` in `<p hidden>` will return `ATTREND|ELEMSTART`)
- accept lowercase `<!doctype `
- accept unquoted attribute value `<form method=GET>`
- accept value-less attribute `<p hidden>`
- (HTML5 mode) threat encoutered [void elements](https://developer.mozilla.org/en-US/docs/Glossary/Void_element) as self-closed
- (HTML5 mode) ignore end-tag of void elements (ex: `</img>`)