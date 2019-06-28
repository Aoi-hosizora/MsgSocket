import { Component, Vue } from 'vue-property-decorator';
import Ren from '@/app/components/Ren/Ren';

@Component({
	components: {
		Ren,
	},
})
export default class About extends Vue {
	// 表单
	protected data() {
		return {
			color: "",
		};
	}
}
