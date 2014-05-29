
# Combine adjacent hashes:

# Step 1: Delete tables if they aready exist
mysql -u root --database="db" --execute="drop table temp_table;"
mysql -u root --database="db" --execute="drop table table_larger_segments;"

# Step 2: Create a tempory table like the orignal table
mysql -u root --database="db" --execute="create table temp_table like main_table;"
# Step 3: Copy the main table to the temporary table
mysql -u root --database="db" --execute="insert into temp_table select * from main_table;"
# Step 4: Create the table for to saved the md5s of combined segments
mysql -u root --database="db" --execute="create table table_larger_segments (val TEXT,size BIGINT);"
numOfSegsToCombine=8;
echo "Combining $numOfSegsToCombine consecutive hashes and rehashing..."
condition=1
# Loop which combines adjacent segments.
while [ "$condition" -ne "0" ]; do
	md5=""
	size=0
	count=$numOfSegsToCombine
	while [ "$count" -ne "0" ]; do
		md5_new=$(mysql -u root -N --database="db" --execute="select md5 from temp_table limit 1;")
		size_new=$(mysql -u root -N --database="db" --execute="select size from temp_table limit 1;")
		mysql -u root --database="db" --execute="delete from temp_table limit 1;"
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
		query="insert into table_larger_segments values (\"$md5\",$size);"
		mysql -u root --database="db" --execute="$query"
	fi;
done

# Remove dump file if already exists
rm /tmp/file1
# Dump the database table into a temp file
mysql -u root --database="db" --execute="select * into outfile '/tmp/file1' fields terminated by ',' lines terminated by '\n' from table_larger_segments;"

# Generate new MD5 hashes:
# Use these combined md5 values to create new md5s
./out 2
echo "With combined blocks"

mysql -u root --database="db" --execute="drop table table_larger_segments_2;"
mysql -u root --database="db" --execute="create table table_larger_segments_2 like table_larger_segments;"
mysql -u root --database="db" --execute="load data infile '/tmp/nextLevel' into table table_larger_segments_2 fields terminated by ',' lines terminated by '\n';"
# Find duplicates and print result
mysql -u root --database="db" --execute="select count(val) from table_larger_segments_2;"
mysql -u root --database="db" --execute="select count(distinct val) from table_larger_segments_2;"
