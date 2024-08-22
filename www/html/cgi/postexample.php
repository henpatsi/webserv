content-type: text/html

<html>
<body>

Welcome <?php echo $_POST["name"]; ?><br>
Your email address is: <?php echo $_POST["email"]; ?><br>
Environment variables: <?php var_dump(getenv());?><br>
      From stdin: <?php echo file_get_contents('php://stdin');?><br>
      From php://input: <?php echo file_get_contents('php://input');?><br>
</body>
</html>
