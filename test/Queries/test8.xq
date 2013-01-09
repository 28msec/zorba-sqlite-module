import module namespace s = "http://www.zorba-xquery.com/modules/sqlite";

let $db := s:connect("")

return {
  variable $isconn := s:is-connected($db);
  variable $result1 := s:execute-update($db, "CREATE TABLE maintable (id INTEGER primary key asc, name TEXT not null, salary REAL)");
  variable $result2 := s:execute-update($db, "INSERT INTO maintable (name, salary) values('Luis', 100.0)");
  variable $result3 := s:execute-update($db, "INSERT INTO maintable (name, salary) values('Juan', 200.0)");
  variable $result4 := s:execute-update($db, "INSERT INTO maintable (name, salary) values('Rodolfo', 300.0)");
  variable $result5 := s:execute-query($db, "SELECT id, name, salary FROM maintable");

  ($isconn, {"Affected Rows" : $result1}, {"Affected Rows" : $result2}, {"Affected Rows" : $result3}, {"Affected Rows" : $result4}, $result5)
}
