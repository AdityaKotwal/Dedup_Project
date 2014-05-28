# Generate the primary database:
g++ -o out Dedup_Project/main.c
./Dedup_Project/out 1
echo "With 4kb blocks"
#Transfer to SQL:
mysql -u root --database="db" --execute="drop table newTable;"
mysql -u root --database="db" --execute="create table newTable (md5 VARCHAR(50), offset BIGINT, size BIGINT, PRIMARY KEY (offset));"
mysql -u root --database="db" --execute="load data infile '/Users/akotwal/Desktop/Dedup_Project/outfile' into table newTable fields terminated by ',' lines terminated by '\n';";
echo "Total number of md5 hashes:"
mysql -u root --database="db" --execute="select count(md5) from newTable;"
echo "Number of distinct md5 hashes:"
mysql -u root --database="db" --execute="select count(distinct md5) from newTable;"

# Combine adjacent hashes:

mysql -u root --database="db" --execute="drop table copyTable;"
mysql -u root --database="db" --execute="drop table nextLevel;"
mysql -u root --database="db" --execute="create table copyTable like newTable;"
mysql -u root --database="db" --execute="insert into copyTable select * from newTable;"
md5_2="md5"
mysql -u root --database="db" --execute="create table nextLevel (val VARCHAR(100),size BIGINT);"
while [ $md5_2 ]; do
	md5_1=$(mysql -u root -N --database="db" --execute="select md5 from copyTable limit 1;")
	size_1=$(mysql -u root -N --database="db" --execute="select size from copyTable limit 1;")
	mysql -u root --database="db" --execute="delete from copyTable limit 1;"
	md5_2=$(mysql -u root -N --database="db" --execute="select md5 from copyTable limit 1;")
	size_2=$(mysql -u root -N --database="db" --execute="select size from copyTable limit 1;")
	mysql -u root --database="db" --execute="delete from copyTable limit 1;"
	md5=$(echo "$md5_1 $md5_2")
	if [ $size_1 ]; then
		if [ $size_2 ]; then
			size=$(($size_1 + $size_2))
		else
			size=$size_1;
		fi
	else
		size=0
	fi

	query="insert into nextLevel values (\"$md5\",$size);"
	mysql -u root --database="db" --execute="$query"
done
rm /tmp/file1
mysql -u root --database="db" --execute="select * into outfile '/tmp/file1' fields terminated by ',' lines terminated by '\n' from nextLevel;"

#Generate new MD% hashes:
./Dedup_Project/out 2
echo "With 8kb blocks"
mysql -u root --database="db" --execute="drop table nextLevelTable;"
mysql -u root --database="db" --execute="create table nextLevelTable like nextLevel;"
mysql -u root --database="db" --execute="load data infile '/tmp/nextLevel' into table nextLevelTable fields terminated by ',' lines terminated by '\n';"
mysql -u root --database="db" --execute="select count(val) from nextLevelTable;"
mysql -u root --database="db" --execute="select count(distinct val) from nextLevelTable;"


