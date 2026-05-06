require("dotenv").config();
const express = require("express");
const mysql = require("mysql2");
const cors = require("cors");

const app = express();
app.use(express.json());
app.use(cors());

// MySQL connection
const db = mysql.createConnection({
    host: process.env.DB_HOST,
    user: process.env.DB_USER,
    password: process.env.DB_PASS,
    database: process.env.DB_NAME
});

db.connect(err => {
    if (err) {
        console.log("Database connection failed:", err);
        return;
    }
    console.log("Database connected!");
});

// API: Get all dustbins
app.get("/bins", (req, res) => {
    db.query("SELECT * FROM dustbins", (err, results) => {
        if (err) return res.status(500).send(err);
        res.json(results);
    });
});

// API: Update dustbin status
app.post("/update", (req, res) => {
    const { id, is_full } = req.body;

    db.query(
        "UPDATE dustbins SET is_full=?, last_updated=NOW() WHERE id=?",
        [is_full, id],
        (err) => {
            if (err) return res.status(500).send(err);
            res.send("Dustbin updated successfully!");
        }
    );
});



// API: Update new dustbin 
app.post("/bins", (req, res) => {
    const { id, location_name, latitude, longitude, is_full } = req.body;
	db.query("SELECT * FROM dustbins WHERE id = ?",[id], (err, result) => {
		if (err) {
        console.log(err);
        return res.status(500).send("Database error");
    };
	if (result.length > 0) {
        
		return res.status(400).send("Please enter unique id. id "+ id + " already exists");
    }
	if (typeof location_name !== "string")return res.status(400).send("Please enter location name in string.");
	db.query(
        "INSERT into dustbins (id, location_name, longitude, latitude ,is_full, last_updated) values (?,?,?,?,?,NOW())",
        [id , location_name, longitude, latitude, is_full],
        (err) => {
            if (err) return res.status(500).send("Failed to Insert");
            return res.send("Dustbin with id "+ id +" added successfully!");
        }
    );
	});
});

// Delete a dustbin
app.delete("/bins",(req, res)=> {
	const {id}= req.body;
	db.query("delete from dustbins where id = ?",
	[id],
	(err, result) => {
			
			if (result.affectedRows === 0) return res.status(404).send('No record with id ' + id + ' found !');
            if (err) return res.status(400).send(err);
            res.send("Dustbin with id " +id + " deleted successfully !");
        }
	);
});

//update bin location
app.put("/location", (req, res) => {
    const { id, location_name } = req.body;
	if (typeof location_name !== "string")return res.status(400).send("Please enter location name in string.");
	

    db.query(
        "UPDATE dustbins SET location_name=?, last_updated=NOW() WHERE id=?",
        [location_name, id],
        (err) => {
            if (err) return res.status(500).send(err);
            res.send("Dustbin location name updated successfully!");
        }
    );
});

//update longitude and latitude
app.put("/coords",(req,res)=>{
	const{id,longitude,latitude} = req.body;
	if(typeof latitude!== "number" || typeof longitude!=="number"){
		return res.status(400).send("latitude and longitude must be in numbers");
}
	db.query(
		"UPDATE dustbins SET latitude=?, longitude=? , last_updated=NOW() WHERE id=?",
		[latitude,longitude,id],
		(err,result) => {
            if (err) return res.status(500).send(err);
			if (result.affectedRows === 0) {
                return res.status(404).send("No dustbin found");
            }
            res.send("Dustbin latitude and longitude updated successfully!");
        }
	);
});


const PORT = process.env.PORT || 3000;
app.listen(3000, '0.0.0.0', () =>
    console.log(`Server running on port ${PORT}`)
);