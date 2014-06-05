#!/bin/sh

#  createMasterTable.sh
#  Dedup_Project
#
#  Created by Aditya Kotwal on 5/30/14.
#  Copyright (c) 2014 Aditya Kotwal. All rights reserved.

# Delete the table if already exists
mysql -u root --database="db" --execute="drop table main_table;"
# Create the table which will hold the MD5 values along with offsets and sizes
mysql -u root --database="db" --execute="create table main_table (md5 VARCHAR(50), offset BIGINT, size BIGINT, PRIMARY KEY (offset));"
# Load the generated CSV file directly into the database
mysql -u root --database="db" --execute="load data infile '/Users/akotwal/Desktop/Dedup_Project/outfile' into table main_table fields terminated by ',' lines terminated by '\n';";
# Display results
echo "Total number of md5 hashes:"
mysql -u root --database="db" --execute="select count(md5) from main_table;"
echo "Number of distinct md5 hashes:"
mysql -u root --database="db" --execute="select count(distinct md5) from main_table;"