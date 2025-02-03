document.addEventListener("DOMContentLoaded", function () {
    fetch("projects.json")
        .then(response => response.json())
        .then(data => {
            const tableBody = document.getElementById("projects-table");
            data.forEach(project => {
                let row = document.createElement("tr");
                row.innerHTML = `
                    <td>${project.name}</td>
                    <td>${project.description}</td>
                    <td>${project.status}</td>
                    <td><a href="${project.link}" target="_blank">View</a></td>
                `;
                tableBody.appendChild(row);
            });
        })
        .catch(error => console.error("Error loading projects:", error));
});