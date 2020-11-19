function register() {
	window.alert("Your registration has been confirmed.\nYou can now book for a seat\n.Remember to bring your ID, protection mask, you will be request to verify your body temperature for any anomalies.");
	document.getElementById("register").innerHTML = "Registration completed";
}

function returnJsonListBook() {
	let dropdown = $('#booklist')
	var newWindows = window.open("list_b.json", "newWindow")
	document.getElementById("Books").innerHTML = "List book opened";
	window.alert("Here, there are the available books in this library!");
}

function book() {
	window.alert("Your registration has been confirmed.\nYou booked for a seat\n.Remember to bring your ID, protection mask, you will be request to verify your body temperature for any anomalies.");
	document.getElementById("book1").innerHTML = "Book completed! You can go correctly to the library!";
}

function bookSeat(n_people) {
	if (n==0) {
		n_people=1;
	}
	else n_people++;
	document.getElementById(l)
}