<template>
	<div id="app">
		<div id="nav">
			<router-link to="/">Home</router-link> |
			<router-link to="/about">About</router-link>
		</div>
		<!-- router-view -->
		<router-view />
	</div>
</template>

<style lang="scss" src="./App.scss"></style>
<script lang="ts" src="./App.ts"></script>