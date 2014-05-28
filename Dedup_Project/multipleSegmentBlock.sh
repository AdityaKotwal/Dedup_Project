# Combine adjacent hashes:
mysql -u root --database="db" --execute="drop table copyTable;"
mysql -u root --database="db" --execute="drop table nextLevel;"
mysql -u root --database="db" --execute="create table copyTable like newTable;"
mysql -u root --database="db" --execute="insert into copyTable select * from newTable;"
mysql -u root --database="db" --execute="create table nextLevel (val TEXT,size BIGINT);"
numOfSegsToCombine=8;
echo "Combining $numOfSegsToCombine consecutive hashes and rehashing..."
condition=1
while [ "$condition" -ne "0" ]; do
	md5=""
	size=0
	count=$numOfSegsToCombine
	while [ "$count" -ne "0" ]; do
		md5_new=$(mysql -u root -N --database="db" --execute="select md5 from copyTable limit 1;")
		size_new=$(mysql -u root -N --database="db" --execute="select size from copyTable limit 1;")
		mysql -u root --database="db" --execute="delete from copyTable limit 1;"
		if [ $md5_new ]; then
			condition=1;
		else
			condition=0;
			break;
		fi
		md5=$(echo "$md5 $md5_new")
		size=$(($size + $size_new))
		count=$(($count - 1))
	done
	if [ "$size" -ne "0" ]; then
		query="insert into nextLevel values (\"$md5\",$size);"
		mysql -u root --database="db" --execute="$query"
	fi;
done
rm /tmp/file1
mysql -u root --database="db" --execute="select * into outfile '/tmp/file1' fields terminated by ',' lines terminated by '\n' from nextLevel;"

#Generate new MD5 hashes:
./out 2
echo "With combined blocks"
mysql -u root --database="db" --execute="drop table nextLevelTable;"
mysql -u root --database="db" --execute="create table nextLevelTable like nextLevel;"
mysql -u root --database="db" --execute="load data infile '/tmp/nextLevel' into table nextLevelTable fields terminated by ',' lines terminated by '\n';"
mysql -u root --database="db" --execute="select count(val) from nextLevelTable;"
mysql -u root --database="db" --execute="select count(distinct val) from nextLevelTable;"
