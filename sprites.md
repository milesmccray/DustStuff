---
layout: default
title: Sprites
body_class: sprites-page
---
<details>
<summary>Sprite Notes</summary>
<div markdown="1">

#### Getting Started With Custom Sprites
First, create a folder called `content_src` in your root Dustforce folder `dustforce/content_src`. Download and extract this [sprite folder](https://drive.google.com/file/d/14aNL_rRM6ORAa3Mj8McvYpzAKKN8llim/view?usp=drive_link) into your newly created `content_src` folder. You should now have `dustforce/content_src/sprites/...` with a bunch of various folders. 

#### Creating Your Own Sprites
Head over to [C's Dustforce Recolor Tool](github.com/cmann1/dustforce-recolour/) and create your own sprites! Once you are done, replace the file in `dustforce/content/sprites` with the exact file you just created. *Note: This is a compiled sprite, all the files in `dustforce/content_src` are non-compiled sprites*. Always backup and sprites or folders you modify before deleting them, but you can always re-grab the default sprites from this website if needed.

#### Compiled Sprites Versus Non-Compiled Sprites
All of Dustforce's compiled sprites live in `/content/sprites`. These are a single file made up of different image frames. Non-compiled sprites are the folder/images you see in `/content_src/sprites`. If a sprite is missing from `/content/sprites` the game looks at the content_src folder for the corresponding images to recompile it. This can be used to create *custom* compiled sprites by first deleting the original sprite from `/content/sprites` and then directly modifying the image frames located in `/content_src/sprites/sprite-to-be-edited`. When you relaunch the game after doing this the game will recompile the sprite (putting it back in `/content/sprites`) but with the new images placed in `/content_src/sprites/`.

#### Using Dustmod To Change Sprites
Instead of replacing sprites directly, you can tell Dustmod to use your chosen sprite skin. To do this, In game navigate to `Dustmod>Display>Sprites` *Note: There are two menus, Characters and Effects, and then one for players, one for bosses*. In the **Character** tab, rename your chosen character's sprite set to the exact name of your compiled sprite in `content/sprites`, such as `dustman-black`. Next navigate to the **Effects** tab and rename your chosen character's sprite set again with the exact name plus the corresponding suffix such as `dustman-black_dm`.

```dustman_dm , vdustman_vdm || dustgirl_dg , vdustgirl_vdg || dustkid_dk , vdustkid_vdk || dustworth_do , vdustworth_vdo || slimeboss_sb , vslimeboss_vsb || trashking_tk , vtrashking_vtk || leafsprite_ls , vleafsprite_vls || dustwraith_dw , vdustwraith_vdw```
</div>
</details>

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