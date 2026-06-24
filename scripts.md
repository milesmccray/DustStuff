---
layout: default
title: Scripts
---

{% assign categories = site.data.scripts | map: "category" | uniq %}

{% for category in categories %}
<h2>{{ category }}</h2>
<table>
  <thead>
    <tr>
      <th>Name</th>
      <th>Description</th>
      <th>Tags</th>
      <th>Download</th>
    </tr>
  </thead>
  <tbody>
    {% for item in site.data.scripts %}
      {% if item.category == category %}
      <tr>
        <td>{{ item.title }}</td>
        <td>{{ item.description }}</td>
        <td>{{ item.tags | join: ", " }}</td>
        <td><a href="{{ item.path | relative_url }}" download>Download</a></td>
      </tr>
      {% endif %}
    {% endfor %}
  </tbody>
</table>
{% endfor %}