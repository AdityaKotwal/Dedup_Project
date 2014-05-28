# Generate the primary database:
g++ -o out main.c
./out 1
echo "With 4kb blocks"

#Transfer to SQL:
mysql -u root --database="db" --execute="drop table newTable;"
mysql -u root --database="db" --execute="create table newTable (md5 VARCHAR(50), offset BIGINT, size BIGINT, PRIMARY KEY (offset));"
mysql -u root --database="db" --execute="load data infile '/Users/akotwal/Desktop/Dedup_Project/outfile' into table newTable fields terminated by ',' lines terminated by '\n';";
echo "Total number of md5 hashes:"
mysql -u root --database="db" --execute="select count(md5) from newTable;"
echo "Number of distinct md5 hashes:"
mysql -u root --database="db" --execute="select count(distinct md5) from newTable;"
./multipleSegmentBlock.sh
