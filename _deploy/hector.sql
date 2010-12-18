-- Service user login information. A slow changing table.
CREATE TABLE tbl_user (
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	login VARCHAR(16) UNIQUE NOT NULL,
	password VARCHAR(40),
	name VARCHAR(255),
	email VARCHAR(255),
	description VARCHAR(255),
	registered DATETIME
);

INSERT INTO tbl_user ( login ) VALUES ( 'amok' );

