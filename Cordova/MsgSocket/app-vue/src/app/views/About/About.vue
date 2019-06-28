<template>
	<div class="about">
		<h1>This is an about page<span v-if="color"> - {{color}}</span></h1>
		<Ren msg="Ren_MSG" v-bind:color="color" />
		<p><input v-model="color" /></p>
		<img src="@/assets/logo.png" />
	</div>
</template>

<style lang="scss" src="./About.scss" scoped></style>
<script lang="ts" src="./About.ts"></script>