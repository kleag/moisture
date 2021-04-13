from django.contrib.auth.models import User

from rest_framework import serializers
from moistureduino.models import Entry


#class EntrySerializer(serializers.Serializer):
#    id = serializers.IntegerField(read_only=True)
#    created = serializers.DateTimeField()
#    kind = serializers.CharField(required=False, allow_blank=True, max_length=100)
#    value = serializers.CharField(required=False, allow_blank=True, max_length=100)
#
#    def create(self, validated_data):
#        """
#        Create and return a new `Entry` instance, given the validated data.
#        """
#        return Entry.objects.create(**validated_data)
#
#    def update(self, instance, validated_data):
#        """
#        Update and return an existing `Entry` instance, given the validated data.
#        """
#        instance.created = validated_data.get('created', instance.created)
#        instance.kind = validated_data.get('kind', instance.kind)
#        instance.value = validated_data.get('value', instance.value)
#        instance.save()
#        return instance

class EntrySerializer(serializers.HyperlinkedModelSerializer):
    owner = serializers.ReadOnlyField(source='owner.username')
    highlight = serializers.HyperlinkedIdentityField(
        view_name='entry-highlight', format='html')
    class Meta:
        model = Entry
        fields = ['url', 'id', 'highlight', 'owner',
                  'created', 'kind', 'value']


class UserSerializer(serializers.HyperlinkedModelSerializer):
    entries = serializers.HyperlinkedRelatedField(many=True, view_name='entry-detail', read_only=True)

    class Meta:
        model = User
        fields = ['url', 'id', 'username', 'entries']

