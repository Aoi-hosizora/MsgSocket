import { Component, Vue, Prop } from 'vue-property-decorator';

@Component
export default class Ren extends Vue {
	@Prop() private msg!: string;
	@Prop() private color!: string;
}
