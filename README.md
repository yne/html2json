Tiny HTML to JSON Converter
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
    ["head",{},[
        ["meta", {"charset": "utf-8"} ],
        ["title", {}, "Basic Example" ],
        ["link", {"rel": "stylesheet"} ]
    ]],
    ["body", {"id": "home"}, [
        ["input", {"type": "text"}]
        ["p", {} ,"content"],
    ]]
]]
```

</td></tr>
</table>