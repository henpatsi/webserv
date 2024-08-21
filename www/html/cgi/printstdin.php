content-type: html/text

<html>
<body>
      <?php
while (FALSE !== ($line = fgets(STDIN))) {
   echo $line;
}
?>
</body>
</html>
