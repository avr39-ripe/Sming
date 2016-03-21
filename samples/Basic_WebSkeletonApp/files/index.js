$( document ).ready(function() {
	
	(function worker() {
		$.getJSON('/state.json', function(data) {
			document.getElementById('counter').textContent = data.counter;
			setTimeout(worker, 5000);
		});
	})();
});