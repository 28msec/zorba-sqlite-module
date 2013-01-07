import module namespace s =
  "http://www.zorba-xquery.com/modules/sqlite";

let $xml := 
<root>
  <food><name>carrot</name><calories>80</calories></food>
  <food><name>tomato</name><calories>45</calories></food>
</root>
let $db := s:connect("")
let $inst := s:execute-update($db, "CREATE TABLE smalltable (id INTEGER primary key asc, name TEXT not null, calories TEXT)")
let $prep-stmt := s:prepare-statement($db, "INSERT INTO smalltable (name, calories) VALUES (?, ?)")

for $e in $xml//food
let $name := $e//name
let $calories := $e//calories
return {
  s:clear-params($prep-stmt);
  s:set-string($prep-stmt, 1, $name);
  s:set-string($prep-stmt, 2, $calories);
  {"Affected Rows" : s:execute-update-prepared($prep-stmt)}
}
