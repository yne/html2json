def getElementByAttr(attr):
    . as $elem |
    if all(attr|keys[]; ($elem[1]//{})[.] == attr[.]) then [.]
    else (.[2]//[]) | map(if type == "array" then getElementByAttr(attr) else null end) | add end;

getElementByAttr({"type":"text"})
