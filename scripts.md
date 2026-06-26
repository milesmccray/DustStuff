---
layout: default
title: Scripts
body_class: scripts-page
---
<details>
<summary>Script Notes</summary>
<div markdown="1">

#### A Note Regarding Dependencies
I recommend grabbing the `lib` and `module` folder and putting them into your `Dustforce/user/script_src` folder. Many scripts included here depend on the files in these folders to function.
Every script here that includes these dependencies "expects" to be within a folder to compile properly. Example `Dustforce/user/script_src/my-new-map/shadows.cpp`. If you don't want to put it in a separate folder, remove the `../` from the `#include` at the top of the script.

#### To Compile
Open up your level in Dustforce and make sure the leveltype is set to dustmod. Dustforce looks for scripts within your `/script_src` folder. So in the earlier example, open up the script tab in your level and type `my-new-map/shadows.cpp`

</div>
</details>

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