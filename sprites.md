---
layout: default
title: Sprites
body_class: sprites-page
---

<style>
.sprite-grid {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(240px, 1fr));
  gap: 16px;
  margin-bottom: 30px;
  padding-left: 16px;
}
.sprite-card {
  border: 2px solid #000;
  border-radius: 6px;
  padding: 14px;
  text-align: center;
  background-color: #f9f9f9;
  display: flex;
  flex-direction: column;
  align-items: center;
}
.sprite-card img {
  max-width: 100%;
  height: 120px;
  object-fit: contain;
  margin-bottom: 8px;
}
.sprite-card h4 {
  margin: 4px 0;
  font-size: 16px;
  white-space: normal;
  overflow-wrap: break-word;
}
.sprite-card p {
  font-size: 13px;
  color: #555;
  margin: 2px 0;
}
.sprite-card a,
.sprite-card button {
  display: block;
  width: 100%;
  text-align: center;
  box-sizing: border-box;
}
</style>
<script src="{{ '/js/preview.js' | relative_url }}"></script>
<div id="preview-modal" class="preview-modal">
  <span class="preview-close" onclick="closePreview()">&times;</span>
  <img id="preview-modal-img" src="" alt="Preview">
</div>

{% assign categories = site.data.sprites | map: "category" | uniq %}
{% for category in categories %}
<h2>{{ category }}</h2>
<div class="sprite-grid">
  {% for item in site.data.sprites %}
    {% if item.category == category %}
    <div class="sprite-card">
		<h4>{{ item.title }}</h4>
		<p>{{ item.author }}</p>
		<a href="{{ item.path | relative_url }}" download>Download</a>
		{% if item.imagePath %}
			<button onclick="openPreview('{{ item.imagePath | relative_url }}')">Preview</button>
		{% endif %}
	</div>
    {% endif %}
  {% endfor %}
</div>
{% endfor %}